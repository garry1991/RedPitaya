#include <stdio.h>
#include <stdlib.h>

#include <redpitaya/rp.h>

#include <sys/time.h>

#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "5000"   // the port users will be connecting to

int main(int argc, char **argv){

   /* Print error, if rp_Init() function failed */
   if(rp_Init() != RP_OK){
      fprintf(stderr, "Rp api init failed!\n");
   }
   
   uint32_t array_size = 16 * 1024; //Current buffer size. 

   float *buff = (float *)malloc(array_size * sizeof(float));

    /* Set up network stuff */
    int sockfd;
   struct addrinfo hints, *servinfo, *p;
   int rv;
   int numbytes;

   if (argc != 3) {
      fprintf(stderr,"usage: talker hostname message\n");
      exit(1);
   }

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_DGRAM;

   if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
   }

   // loop through all the results and make a socket
   for(p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
         perror("talker: socket");
         continue;
      }

      break;
   }

   if (p == NULL) {
      fprintf(stderr, "talker: failed to bind socket\n");
      return 2;
   }
    /* Finished setting up network stuff */
    rp_AcqReset();

    rp_pinState_t gain_state = RP_HIGH;

    rp_AcqSetGain(RP_CH_1, gain_state);
    rp_AcqSetGain(RP_CH_2, gain_state);

    rp_AcqGetGain(RP_CH_2, &gain_state);
    if (gain_state == RP_LOW){
        printf("Low\n");
    } else {
        printf("High\n");
    }

    rp_AcqSetTriggerLevel(0.5);

   /* Starts acquisition */
    rp_AcqSetSamplingRate(RP_SMP_125M);
   rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
   rp_acq_trig_state_t state = RP_TRIG_STATE_WAITING;

    uint32_t decimation_factor;
    rp_acq_decimation_t decimation = RP_DEC_1;
    rp_AcqSetDecimation(decimation);
    rp_AcqGetDecimation(&decimation_factor);
    printf("Decimation factor: %d\n", decimation_factor);

    int32_t trigger_delay;
    rp_AcqGetTriggerDelay(&trigger_delay);
    printf("Trigger delay: %d\n", trigger_delay);
    
    int loops = 1;
    
    uint32_t write_pointer;

    for(int j=loops; j > 0; j--) {
        rp_AcqStart();    

        while (state == RP_TRIG_STATE_WAITING){
            rp_AcqGetTriggerState(&state);

            if(state == RP_TRIG_STATE_TRIGGERED){
                state = RP_TRIG_STATE_WAITING;
                printf("triggered\n");
                break;
                
            } else {
                printf("not triggered\n");
            }
        }
    }

    uint32_t get_size = array_size;
    
    printf("request size: %d\n", get_size);                
    rp_AcqGetLatestDataV(RP_CH_2, &get_size, buff);
    rp_AcqGetWritePointerAtTrig(&write_pointer);
    printf("out size: %d\n", get_size);
    printf("write pointer: %d\n", write_pointer);

    for(int n=0; n<16; n++){
        if ((numbytes = sendto(sockfd, &buff[n*1024], 1024 * sizeof(float), 0,
                        p->ai_addr, p->ai_addrlen)) == -1) {
            perror("talker: sendto");
            exit(1);
        }
    }
    
    /* free network resources */
    freeaddrinfo(servinfo);
   close(sockfd);

   /* Releasing red pitaya resources */
   free(buff);
   rp_Release();



   return 0;
}

        
