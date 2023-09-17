#include "esphome.h"

class UartTrimetricSensor : public Component, public UARTDevice, public TextSensor {
 public:
  UartTrimetricSensor(UARTComponent *parent) : UARTDevice(parent) {}

  Sensor *s_V = new Sensor();
  Sensor *s_FV = new Sensor();
  Sensor *s_A = new Sensor();
  Sensor *s_FA = new Sensor();
  Sensor *s_AH = new Sensor();
  Sensor *s_percent = new Sensor();
  Sensor *s_W = new Sensor();
  Sensor *s_DSC = new Sensor();
  Sensor *s_DSE = new Sensor();
  Sensor *s_FW = new Sensor();
  Sensor *s_W_in = new Sensor();
  Sensor *s_W_out = new Sensor();

  void setup() override {
    // nothing to do here
  }

  int readline(int readch, char *buffer, int len)
  {
    static int pos = 0;
    int rpos;

    if (readch > 0) {
      switch (readch) {
        case '%': 
          rpos = pos;
          pos = 0;  // Reset position index ready for next time
          return rpos;
        default:
          if (pos < len-1) {
            buffer[pos++] = readch;
            buffer[pos] = 0;
          }
      }
    }
    // No end of line has been found, so return -1.
    return -1;
  }

  int readbuffer(int readch, char *buffer, int len)
  {
    static int pos = 0;
    int rpos;
    
    if (pos < len-1) {
      buffer[pos++] = readch;
      buffer[pos] = 0;
    }
    if (pos == len-1) {
      rpos = pos;
      pos = 0;  // Reset position index ready for next time
      return rpos;
    }
    // buffer not full, so return -1.
    return -1;
  }

  bool strbegin(const char *pre, const char *str)
  {
      return strncmp(pre, str, strlen(pre)) == 0;
  }

  void publish_states(char *segment) 
  {
    char *token;
    char V_token[10]="";
    char FV_token[10]="";
    char A_token[10]="";
    char FA_token[10]="";
    char AH_token[10]="";
    char percent_token[10]="";
    char W_token[10]="";
    char DSC_token[10]="";
    char DSE_token[10]="";
    float V_value=0;
    float FV_value=0;
    float A_value=0;
    float FA_value=0;
    float AH_value=0;
    float percent_value=0;
    float W_value=0;
    float DSC_value=0;
    float DSE_value=0;
    
    ESP_LOGD("custom", "segment: %s", segment);
    // A line should look like this V=25.3,FV=25.2,V2=00.0,A=00.5,FA=00.4,PW=2FF,AH=-0.08,%=100,W=12.1,DSC=3.84,DSE=3.84,PW=2FF
    // periodically you will get    V=25.3,FV=25.2,V2=00.0,A=00.5,FA=00.4,PW=2FF,AH=-0.08,%=100,W=12.1,DSC=3.84,DSE=3.84,PW=2FF,X=28A

    token = strtok(segment, ",");
    /* walk through other tokens */
    while( token != NULL ) {
      
      if (strbegin("V=", token)) {
        ESP_LOGD("custom", "V token: %s", token);
        strcpy(V_token, token);
      } else if(strbegin("FV=", token)) {
        ESP_LOGD("custom", "FV token: %s", token);
        strcpy(FV_token, token);
      } else if(strbegin("A=", token)) {
        ESP_LOGD("custom", "A token: %s", token);
        strcpy(A_token, token);
      } else if(strbegin("FA=", token)) {
        ESP_LOGD("custom", "FA token: %s", token);
        strcpy(FA_token, token);
      } else if(strbegin("AH=", token)) {
        ESP_LOGD("custom", "AH token: %s", token);
        strcpy(AH_token, token);
      } else if(strbegin("%=", token)) {
        ESP_LOGD("custom", "percent token: %s", token);
        strcpy(percent_token, token);
      } else if(strbegin("W=", token)) {
        ESP_LOGD("custom", "W token: %s", token);
        strcpy(W_token, token);
      } else if(strbegin("DSC=", token)) {
        ESP_LOGD("custom", "DSC token: %s", token);
        strcpy(DSC_token, token);
      } else if(strbegin("DSE=", token)) {
        ESP_LOGD("custom", "DSE token: %s", token);
        strcpy(DSE_token, token);
      }
      token = strtok(NULL, ",");
    }
    token = strtok(V_token, "=");
    V_value = strtof(strtok(NULL, ","), NULL);
    ESP_LOGD("custom", "V value: %f", V_value);

    token = strtok(FV_token, "=");
    FV_value = strtof(strtok(NULL, ","), NULL);
    ESP_LOGD("custom", "FV value: %f", FV_value);

    token = strtok(A_token, "=");
    A_value = strtof(strtok(NULL, ","), NULL);
    ESP_LOGD("custom", "A value: %f", A_value);

    token = strtok(FA_token, "=");
    FA_value = strtof(strtok(NULL, ","), NULL);
    ESP_LOGD("custom", "FA value: %f", FA_value);

    token = strtok(AH_token, "=");
    AH_value = strtof(strtok(NULL, ","), NULL);
    ESP_LOGD("custom", "AH value: %f", AH_value);

    token = strtok(percent_token, "=");
    percent_value = strtof(strtok(NULL, ","), NULL);
    ESP_LOGD("custom", "percent value: %f", percent_value);

    token = strtok(W_token, "=");
    W_value = strtof(strtok(NULL, ","), NULL);
    ESP_LOGD("custom", "W value: %f", W_value);

    token = strtok(DSC_token, "=");
    DSC_value = strtof(strtok(NULL, ","), NULL);
    ESP_LOGD("custom", "DSC value: %f", DSC_value);

    token = strtok(DSE_token, "=");
    DSE_value = strtof(strtok(NULL, ","), NULL);
    ESP_LOGD("custom", "DSE value: %f", DSE_value);

    s_V->publish_state(V_value);
    //s_FV->publish_state(FV_value);
    s_FV->publish_state(V_value);
    s_A->publish_state(A_value);
    //s_FA->publish_state(FA_value);
    s_FA->publish_state(A_value);
    s_AH->publish_state(AH_value);
    s_percent->publish_state(percent_value);
    s_W->publish_state(V_value*A_value);
    s_DSC->publish_state(DSC_value);
    s_DSE->publish_state(DSE_value);
    //s_FW->publish_state(FV_value*FA_value);
    s_FW->publish_state(V_value*A_value);
    if (V_value*A_value > 0) {
      s_W_in->publish_state(V_value*A_value);
    } else {
      s_W_in->publish_state(0);
    }
    if (V_value*A_value < 0) {
      s_W_out->publish_state(fabs(V_value*A_value));
    } else {
      s_W_out->publish_state(0);
    }
  }

  void loop() override {
    const int max_line_length = 80;
    static char rawbuffer[max_line_length+1];
    static char streambuffer[(max_line_length*2)+1] = "";
    static char tailbuffer[(max_line_length*2)+1] = "";
    static char segment[128] = "";
    char *startsegment;
    char *endsegment;
    int numread;

    //ESP_LOGD("custom", "rawbuffer: %s", rawbuffer);
    //ESP_LOGD("custom", "streambuffer: %s", streambuffer);

    while (available()) {
      numread = readbuffer(read(), rawbuffer, max_line_length);
      //ESP_LOGD("custom", "numread: %i", numread);
      if(numread == max_line_length-1) {
        numread = -1;
        strcat(streambuffer, rawbuffer);
        //ESP_LOGD("custom", "streambuffer: %s", streambuffer);

        startsegment = strstr(streambuffer, ",V=");
        if (startsegment) {  // we have a start of data
          //ESP_LOGD("custom", "startsegment: %s", startsegment);
          
          // check for next start
          endsegment = strstr(startsegment+1, ",V=");
          if (endsegment) {  // if we have a next start then we have a full segment
            //ESP_LOGD("custom", "endsegment: %s", endsegment);

            strncpy(segment,startsegment+1, strlen(startsegment+1)-strlen(endsegment));
            segment[strlen(startsegment+1)-strlen(endsegment)] = '\0';
            //ESP_LOGD("custom", "segment: %s", segment);
            //ESP_LOGD("custom", "sl: %i, el: %i, l: %i", strlen(startsegment+1),strlen(endsegment),strlen(startsegment+1)-strlen(endsegment));
           
            publish_states(segment);
      
            strcpy(tailbuffer, endsegment);
            //ESP_LOGD("custom", "tailbuffer: %s", tailbuffer);

            strcpy(streambuffer, tailbuffer);
          }
          
        }
      }
    }
  }
};