#include "udp_tester.h"

#if LWIP_UDP
#include <stdio.h>
#include <stdlib.h>
#include "lwip/udp.h"
#include "lwip/timeouts.h"
#include "lwip/debug.h"
#include "tim.h"
#include "gpio.h"
#include "math.h"
#include "lwjson/lwjson.h"

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
int counter = 0;
u8_t GoZeroFlag = 0;
float rot_deg = 0;
u8_t rampFlag = 0;
int RPMGlobal;
int PeriodGlobal;
int Period = 0;

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

void direction(u8_t dir);
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



void
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
    int op2;
    u16_t value;
} UdpPack;
/*
typedef struct {
    char* addrr;
} UdpPacker;
*/
static void
recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	GoZeroFlag = 0;

	UdpPack *sbuf = (UdpPack*) p->payload;

    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(upcb);
/*
   if (((tftp_state.port != 0) && (port != tftp_state.port)) ||
      (!ip_addr_isany_val(tftp_state.addr) && !ip_addr_cmp(&tftp_state.addr, addr))) {
    send_msg(addr, port, "Only one connection at a time is supported");
    pbuf_free(p);
    return;
    }
*/
   //snprintf()

   tftp_state.last_pkt = tftp_state.timer;
   tftp_state.retries = 0;

   ip_addr_t toaddr;
   toaddr.addr = 0x7801a8c0; // PC

   /*
   ip_addr_t broadcast;
   broadcast.addr = 0xff01a8c0; // broadcast

   ip_addr_t SrcAddr;
   SrcAddr.addr = ip_current_src_addr(); // broadcast
   char ipsrc = ip_current_src_addr();

   */

   int hejmed1 = ip_addr_isbroadcast(ip_current_dest_addr(), ip_current_netif());

   if(ip_addr_isbroadcast(ip_current_dest_addr(), ip_current_netif()) == 1){
	   send_msg(&toaddr, 73, "Jeg er fork board");

   }

   else {
	   if(sbuf -> addrr == 1){
	   		//send_msg(addr, port, "LED Toggle");
	   	  // toaddr.port = 70


	   		send_msg(&toaddr, 73, "hej");
	   		for(int i=0; i <= sbuf -> op1; i++){
	   			HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13); // A7
	   			HAL_Delay(500);
	   		}
	   	}
	   	else if(sbuf -> addrr == 2){
	   		send_msg(addr, 73, "vroom vroom");

	   		if(sbuf -> op1 == 2){
	   			__HAL_TIM_SET_AUTORELOAD(&htim2,30000);
	   			//ReturnToZero();
	   		}

	   		else {

	   			rot_deg = roundf(sbuf-> op2 * 1.111)*8;

	   			//direction(sbuf->op1);

	   			//calcPeriod(sbuf -> value);
	   		}

	   	}
	   	else if(sbuf -> addrr == 3){
	   		send_msg(addr, 73, "Gaffel sensor");
	   	}
	   	else if (sbuf -> addrr == '4'){
	   		send_msg(&toaddr, 73, "Skulle hilse fra board 1");
	   	}
	   	else{
	   	  send_msg(addr, 73, "Invalid key. Try again");
	   	  //HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
	   	}
   }
    pbuf_free(p);
}

/* LwJSON instance and tokens */
static lwjson_token_t tokens[128];
static lwjson_t lwjson;

void
example_minimal_run(void) {
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
    if (lwjson_parse(&lwjson, "{\"mykey\":\"myvalue\"}") == lwjsonOK) {
        const lwjson_token_t* t;
        printf("JSON parsed..\r\n");

        /* Find custom key in JSON */
        if ((t = lwjson_find(&lwjson, "mykey")) != NULL) {
            printf("Key found with data type: %d\r\n", (int)t->type);
        }

        /* Call this when not used anymore */
        lwjson_free(&lwjson);
    }
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
  //err_t ret1;

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

 /* ret1 = udp_bind(pcb, IP_ANY_TYPE, TFTP_PORT);
   if (ret != ERR_OK) {
     udp_remove(pcb);
     return ret;
   }*/

 tftp_state.handle    = NULL;
  tftp_state.port      = 0;
  //tftp_state.ctx       = ctx;
  tftp_state.timer     = 0;
  tftp_state.last_data = NULL;
  tftp_state.upcb      = pcb;

 udp_recv(pcb, recv, NULL);

 ip_addr_t toaddr;
 toaddr.addr = 0x7801a8c0; // PC
 send_msg(&toaddr, 73, "hej");

 return ERR_OK;

}


void tftp_cleanup(void)
{
  LWIP_ASSERT("Cleanup called on non-initialized TFTP", tftp_state.upcb != NULL);
  udp_remove(tftp_state.upcb);
  close_handle();
  memset(&tftp_state, 0, sizeof(tftp_state));
}

/*
void direction(u8_t dir) // 4x each direction
{

	counter = 0;

	if(dir == 1)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	}

	if(dir == 0)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	 }
}
*/

int top_counter = 0;
u8_t rampD_flag = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim==&htim2)
	{
		/*if(GoZeroFlag == 1)
		{
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
			counter = 0;
		}
		else if(GoZeroFlag == 0)
		{
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
			counter += 1;

		   if(RPMGlobal > 300)
		   {
			   if(rampD_flag == 0)
			   {
				   if(counter % 200 == 0)
				   {
					   if(PeriodGlobal > Period)
					   {
							PeriodGlobal -= 5;
							top_counter = counter;
					   }
					   else if(PeriodGlobal == Period)
					   {
						   rampD_flag = 1;
					   }
					   __HAL_TIM_SET_COUNTER(&htim2, 0);
					   __HAL_TIM_SET_AUTORELOAD(&htim2,PeriodGlobal);
				   }
			   }
			   else if(rampD_flag == 1)
			   {
				   if(counter % 200 == 0)
				   {
					  if(counter > (rot_deg - top_counter))
					  {
						  PeriodGlobal += 5;
						  __HAL_TIM_SET_COUNTER(&htim2, 0);
						  __HAL_TIM_SET_AUTORELOAD(&htim2,PeriodGlobal);
					  }
				   }
			   }
			}

			if (counter == rot_deg)
			{
				HAL_TIM_Base_Stop_IT(&htim2);
				__HAL_TIM_SET_COUNTER(&htim2, 0);
				 rampD_flag = 0;
			}
		}*/
	}
}

/*
void calcPeriod(u16_t RPM)
{
	Period = 48000000.0/((RPM/60.0)*1600.0);
	//PeriodGlobal = Period;
	if(RPM > 300)
	{
		__HAL_TIM_SET_AUTORELOAD(&htim2,7800);
		PeriodGlobal = 7800;
		RPMGlobal = RPM;
	} else
	{
		__HAL_TIM_SET_AUTORELOAD(&htim2,Period);
		RPMGlobal = RPM;
	}

	HAL_TIM_Base_Start_IT(&htim2);
}

void ReturnToZero(){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	HAL_TIM_Base_Start_IT(&htim2);
	GoZeroFlag = 1;
}
*/

#endif /* LWIP_UDP */
