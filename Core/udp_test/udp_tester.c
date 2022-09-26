#include "udp_tester.h"

#if LWIP_UDP

#include "lwip/udp.h"
#include "lwip/timeouts.h"
#include "lwip/debug.h"

#define TFTP_MAX_PAYLOAD_SIZE 512
#define TFTP_HEADER_LENGTH    4

#define TFTP_RRQ   1
#define TFTP_WRQ   2
#define TFTP_DATA  3
#define TFTP_ACK   4
#define TFTP_ERROR 5



/*
enum tftp_error {
  TFTP_ERROR_FILE_NOT_FOUND    = 1,
  TFTP_ERROR_ACCESS_VIOLATION  = 2,
  TFTP_ERROR_DISK_FULL         = 3,
  TFTP_ERROR_ILLEGAL_OPERATION = 4,
  TFTP_ERROR_UNKNOWN_TRFR_ID   = 5,
  TFTP_ERROR_FILE_EXISTS       = 6,
  TFTP_ERROR_NO_SUCH_USER      = 7
};
*/



#include <string.h>
 TIM_HandleTypeDef htim1;
int angle = 0;
int angleAdd = 0;

struct tftp_state {
  const struct tftp_context *ctx;
  void *handle;
  struct pbuf *last_data;
  struct udp_pcb *upcb;
  ip_addr_t addr;
  u16_t port;
  int timer;
  int last_pkt;
  u16_t blknum;
  u8_t retries;
  u8_t mode_write;
};

void direction(int dir);
void setFreq(int del);
void quarterPrStep();
void delay(uint16_t us);
static struct tftp_state tftp_state;

static void tftp_tmr(void *arg);

static void
close_handle(void)
{
  tftp_state.port = 0;
  ip_addr_set_any(0, &tftp_state.addr);

 if (tftp_state.last_data != NULL) {
    pbuf_free(tftp_state.last_data);
    tftp_state.last_data = NULL;
  }

 sys_untimeout(tftp_tmr, NULL);

 if (tftp_state.handle) {
    tftp_state.ctx->close(tftp_state.handle);
    tftp_state.handle = NULL;
    LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("tftp: closing\n"));
  }
}



static void
send_msg(const ip_addr_t *addr, u16_t port, const char *str)
{
  int str_length = strlen(str);
  struct pbuf *p;
  u16_t *payload;



 p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)(str_length), PBUF_RAM);
  if (p == NULL) {
    return;
  }

 payload = (u16_t *) p->payload;
  /*
  payload[0] = PP_HTONS(TFTP_ERROR);
  payload[1] = lwip_htons(code);
  */
  MEMCPY(&payload[0], str, str_length);

 udp_sendto(tftp_state.upcb, p, addr, port);
  pbuf_free(p);
}


typedef struct {
    u8_t addrr;
    u8_t op1;
    u8_t op2;
    int value;
} UdpPack;

static void
recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	UdpPack *sbuf = (UdpPack*) p->payload;

    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(upcb);

   if (((tftp_state.port != 0) && (port != tftp_state.port)) ||
      (!ip_addr_isany_val(tftp_state.addr) && !ip_addr_cmp(&tftp_state.addr, addr))) {
    send_msg(addr, port, "Only one connection at a time is supported");
    pbuf_free(p);
    return;
    }

   tftp_state.last_pkt = tftp_state.timer;
   tftp_state.retries = 0;

   if(sbuf -> addrr == 1){
    	//et = (char)*sbuf;
		send_msg(addr, port, "LED Toggle");
		for(int i=0; i <= sbuf -> op1; i++){
			HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13); // A7
			HAL_Delay(500);
		}
    }
    else if(sbuf -> addrr == 2){
    	send_msg(addr, port, "vroom vroom");
    	direction(sbuf->op1);
    }
    else if(sbuf -> addrr == 3){
    	send_msg(addr, port, "Gaffel sensor");
    }
    else{
      send_msg(addr, port, "Invalid key. Try again");
      //HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
    }
    pbuf_free(p);
}



static void
tftp_tmr(void *arg)
{
  LWIP_UNUSED_ARG(arg);

 tftp_state.timer++;

 if (tftp_state.handle == NULL) {
    return;
  }
 sys_timeout(TFTP_TIMER_MSECS, tftp_tmr, NULL);

 if ((tftp_state.timer - tftp_state.last_pkt) > (TFTP_TIMEOUT_MSECS / TFTP_TIMER_MSECS)) {
    if ((tftp_state.last_data != NULL) && (tftp_state.retries < TFTP_MAX_RETRIES)) {
      LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("tftp: timeout, retrying\n"));
      //resend_data();
      tftp_state.retries++;
    } else {
      LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("tftp: timeout\n"));
      close_handle();
    }
  }
}



/** @ingroup tftp
* Initialize TFTP server.
* @param ctx TFTP callback struct
*/
err_t
tftp_init()
{
  err_t ret;

 /* LWIP_ASSERT_CORE_LOCKED(); is checked by udp_new() */
  struct udp_pcb *pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
  if (pcb == NULL) {
    return ERR_MEM;
  }

 ret = udp_bind(pcb, IP_ANY_TYPE, TFTP_PORT);
  if (ret != ERR_OK) {
    udp_remove(pcb);
    return ret;
  }

 tftp_state.handle    = NULL;
  tftp_state.port      = 0;
  //tftp_state.ctx       = ctx;
  tftp_state.timer     = 0;
  tftp_state.last_data = NULL;
  tftp_state.upcb      = pcb;

 udp_recv(pcb, recv, NULL);

 return ERR_OK;
}

/** @ingroup tftp
* Deinitialize ("turn off") TFTP server.
*/
void tftp_cleanup(void)
{
  LWIP_ASSERT("Cleanup called on non-initialized TFTP", tftp_state.upcb != NULL);
  udp_remove(tftp_state.upcb);
  close_handle();
  memset(&tftp_state, 0, sizeof(tftp_state));
}

void setFreq(int del) // sets the frequency
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
  delay(del);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
  delay(del);

}

void direction(int dir) // 4x each direction
{
	if(dir == 1)
	{
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	  for(int i=0; i<800; i++)
	  {
		  setFreq(31000);
	  }
	  dir = 0;
	  HAL_Delay(1000);
	}
	if(dir == 0)
	{
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	  for(int i=0; i<800; i++)
	  {
		  setFreq(31000);
	  }
	  HAL_Delay(1000);
	}
}

void quarterPrStep()
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
  for(int i=0; i<4; i++)
  {

	  for(int j=0; j<50; j++)
	  {

		  setFreq(62000);
		  //angle = 360/(200/j);
	  }
	  //angleAdd += angle;
	  //sprintf(angle2char, "%i", angleAdd);
	  //sprintf(angleData, "%s", angle2char);
	  //HAL_UART_Transmit(&huart1, AngleStr, 7, 10);
	  //HAL_UART_Transmit(&huart1, angleData, 4, 10);
	  //HAL_UART_Transmit(&huart1, newL, 2, 10);
	  HAL_Delay(500);
	  /*if(angleAdd == 360)
	  {
		  angleAdd = 0;
	  }*/
  }
}

void delay(uint16_t us) //delay function in us
 {
 	__HAL_TIM_SET_COUNTER(&htim1, 0);  // sets counter to 0
 	while( __HAL_TIM_GET_COUNTER(&htim1) < us ); // wait for the counter to reach the entered value
 }

#endif /* LWIP_UDP */
