
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iostream>
#include <string>
#include <sstream>
#include <queue>
#include <stack>
#include <pthread.h>


#include "include/ThostFtdcUserApiDataType.h"
#include "include/ThostFtdcTraderApi.h"
#include "include/ThostFtdcUserApiStruct.h"
#include "include/ThostFtdcMdApi.h"

using namespace std;


#define response_size 1024

#define BUFSIZE 1024

//////////////////////////////////Initial Thread//////////////////////////////////

int TCP_server_to_send_socket,TCP_server_socket, newSocket;
char buffer[1024];

// This variables will be initilized by config file
char str[1024],TCP_IP_Address[24],TCP_UDP_IP_Adrress[20]; 

int TCP_or_UDP,TCP_Port,TCP_UDP_Port, Market_or_Order;

struct sockaddr_in TCP_recv_serverAddr,TCP_recv_serverStorage; // For TCP_recv_server

struct sockaddr_in TCP_send_serverAddr,TCP_send_serverStorage; // For TCP_send_server

socklen_t TCP_send_addr_size,TCP_recv_addr_size;

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER, Mutex_UDP = PTHREAD_MUTEX_INITIALIZER, Mutex_Recv = PTHREAD_MUTEX_INITIALIZER, Mutex_Market = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t  condition_print  = PTHREAD_COND_INITIALIZER, condition_recv  = PTHREAD_COND_INITIALIZER, condition_var  = PTHREAD_COND_INITIALIZER, condition_Market_start  = PTHREAD_COND_INITIALIZER;

char IPAddress[100],BrokerID[100],UserID[100],Password[100],InvestorID[100];

void Read_from_Config_File(){
    FILE *file = fopen("Market.cfg","r");
    //file = fopen( "D:\\QuantoTrade_Debug\\Sample.txt","r");
    char input[50];
    if ( file == NULL)
    {
        perror("Error");
        return;
    }

    #ifdef WINDOWS
        fscanf_s(file,"%s",input);

        fscanf_s(file,"%s %s",input,input);
        fscanf_s(file,"%d",&  Market_or_Order);

        fscanf_s(file,"%s %s",input,input);
        fscanf_s(file,"%s",IPAddress);
        
        fscanf_s(file,"%s %s",input,input);
        fscanf_s(file,"%s",BrokerID);
        
        fscanf_s(file,"%s %s",input,input);
        fscanf_s(file,"%s",InvestorID);        

        fscanf_s(file,"%s %s",input,input);
        fscanf_s(file,"%s",UserID);
        
        fscanf_s(file,"%s %s",input,input);
        fscanf_s(file,"%s",Password);


    #else
        fscanf(file,"%s",input);
        fscanf(file,"%s %s",input,input);
        fscanf(file,"%d",&Market_or_Order);
        
        fscanf(file,"%s %s",input,input);
        fscanf(file,"%s",IPAddress);
        
        fscanf(file,"%s %s",input,input);
        fscanf(file,"%s",BrokerID);

        fscanf(file,"%s %s",input,input);
        fscanf(file,"%s",InvestorID);        

        fscanf(file,"%s %s",input,input);
        fscanf(file,"%s",UserID);

        fscanf(file,"%s %s",input,input);
        fscanf(file,"%s",Password);
    
        /*Reading from config file*/
        fscanf(file,"%s %s %d",str,str,&TCP_or_UDP);
        fscanf(file,"%s %s %s",str,str,TCP_IP_Address);
        fscanf(file,"%s %s %d",str,str,&TCP_Port);
        fscanf(file,"%s %s %s",str,str,TCP_UDP_IP_Adrress);
        fscanf(file,"%s %s %d",str,str,&TCP_UDP_Port);
        printf("Port Number:- %d it's TCP port, TCP_IP_Address is %s and it's used to receive the request from the client!!!!!!\n",TCP_Port,TCP_IP_Address);
        if(!TCP_or_UDP){
            printf("Port Number:- %d it's TCP port, IP_Address is %s and it's used to Send the response to the client!!!!!!\n",TCP_UDP_Port,TCP_UDP_IP_Adrress);
        }
        else{
            printf("Port Number:- %d it's UDP port , UDP_IP_Address is %s and it's used to send the response to the client!!!!!!\n",TCP_UDP_Port,TCP_UDP_IP_Adrress);
        }

    #endif
        fclose(file);
}

struct CTPObj
{
    string  ReqBeginString;
    string  ReqLength;
    string  ReqOrderReqType;
    string  ReqRequestID;
    string  ReqSenderCompID;
    string  ReqSendingTime;
    string  ReqTargetCompID;
    string  ReqAccountID;
    string  ReqOrderRef;
    string  ReqConst;
    string  ReqLimitPrice;
    string  ReqOrderPriceType;
    string  ReqOrderQty;
    string  ReqDirection;
    string  ReqInstrumentID;
    string  ReqTransactTime;
    string  ReqFUT;
    string  ReqExpiryDate;
    string  ReqExchangeID;
    string  ReqChecksum;

    string  OnRspAccountID;  //1  
    string  OnRspAvgTime;  //6  
    string  OnRspOrderRef;  //11  
    string  OnRspVolumeTraded;  //14  
    string  OnRspLastPartiallyFilled;  //32  
    string  OnRspRequestID;  //37  
    string  OnRspVolumeTotalOriginal;  //38  
    string  OnRspOrderStatusType;  //39  
    /*     0 - New (THOST_FTDC_OST_Touched)
         1 - Partially Filled (THOST_FTDC_OST_PartTradedQueueing)
         2 - Filled (THOST_FTDC_OST_AllTraded)
         3 - Done for the Day
         4 - Cancel (THOST_FTDC_OST_Canceled)
         5 - Replaced 
         6 - Pending Cancel
         7 - Stopped
         8 - Reject
    */
    string  OnRspOrderType;  //54  
    string  OnRspCUSIP;  //48  
    string  OnRspExecutionTime;  //60  
    string  OnRspMarketOrderPrice;  //44  
    string  OnRspPendingQuantity;  //151  //**Orginal - Traded**//
    string  OnRspExchange;  //207  



    string  MarketInstrumentID;

    /******Market***/
    string AskPrice1; 
    string AskPrice2;
    string AskPrice3;
    string AskPrice4;
    string AskPrice5;

    string AskVolume1;
    string AskVolume2;
    string AskVolume3;
    string AskVolume4;
    string AskVolume5;

    string BidPrice1;
    string BidPrice2;
    string BidPrice3;
    string BidPrice4;
    string BidPrice5;

    string BidVolume1;
    string BidVolume2;
    string BidVolume3;
    string BidVolume4;
    string BidVolume5;

};
typedef struct CTPObj CTPStructure ;

queue<string> QUEUE_FIX_SEND,QUEUE_FIX_RECEIVED;
queue <CTPObj> QUEUE_CTPObj_RECEIVED,QUEUE_CTPObj_SEND;

string FixINString,FixOUTString;
CTPStructure ObjIN,ObjOUT;
//#include "Shanghai_market.h"

void printObject(CTPStructure Object)
{
    /****************************************************/
    printf("BeginString:%s\t\n", Object.ReqBeginString.c_str());
    printf("Length:%s\t\n", Object.ReqLength.c_str());
    printf("OrderReqType:%s\t\n", Object.ReqOrderReqType.c_str());
    printf("RequestID:%s\t\n", Object.ReqRequestID.c_str());
    printf("SenderCompID:%s\t\n", Object.ReqSenderCompID.c_str());
    printf("SendingTime:%s\t\n", Object.ReqSendingTime.c_str());
    printf("TargetCompID:%s\t\n", Object.ReqTargetCompID.c_str());
    printf("AccountID:%s\t\n", Object.ReqAccountID.c_str());
    printf("OrderRef:%s\t\n", Object.ReqOrderRef.c_str());
    printf("Const:%s\t\n", Object.ReqConst.c_str());
    printf("LimitPrice:%s\t\n", Object.ReqLimitPrice.c_str());
    printf("OrderPriceType:%s\t\n", Object.ReqOrderPriceType.c_str());
    printf("OrderQty:%s\t\n", Object.ReqOrderQty.c_str());
    printf("Direction:%s\t\n", Object.ReqDirection.c_str());
    printf("InstrumentID:%s\t\n", Object.ReqInstrumentID.c_str());
    printf("TransactTime:%s\t\n", Object.ReqTransactTime.c_str());
    printf("FUT:%s\t\n", Object.ReqFUT.c_str());
    printf("ExpiryDate:%s\t\n", Object.ReqExpiryDate.c_str());
    printf("Exchange:%s\t\n", Object.ReqExchangeID.c_str());
    printf("Checksum:%s\t\n", Object.ReqChecksum.c_str());
    /****************************************************/
}


class StringSparse
{
    public:
    CTPStructure convertFromFixToCTPObj(string FixString);
    string convertFromCTPObjToFix(CTPStructure);
    private:
    void convertToObj(int tagNum,string message);
    CTPStructure ConvertedObj;
    string ConvertedString;
};
StringSparse ThreadIN,ThreadOUT;

CTPStructure StringSparse::convertFromFixToCTPObj(string FixString){
    std::istringstream iss(FixString);
    std::string token,tagToken,tagMessage;
    while (std::getline(iss, token, '\001'))
    {
        std::istringstream iss(token);
        std::getline(iss, tagToken, '=');
        std::getline(iss, tagMessage, '=');
        int tagNum=atoi(tagToken.c_str());
        convertToObj(tagNum,tagMessage.c_str());
    }
    return ConvertedObj;
}
string StringSparse::convertFromCTPObjToFix(CTPStructure CTPObject)
{ 
    std::string ConvertedString="";

    ConvertedString = ConvertedString + "8="   + "FIX.4.2"    +  "\001";                 
    ConvertedString = ConvertedString + "9="   + "130"        +  "\001"; 
    ConvertedString = ConvertedString + "35="   + "8"        +  "\001";              
    if(Market_or_Order)
    {
        if ( CTPObject.OnRspAccountID           !=""  ) {  ConvertedString = ConvertedString + "1="   + CTPObject.OnRspAccountID           +  "\001"; }
        if ( CTPObject.OnRspAvgTime             !=""  ) {  ConvertedString = ConvertedString + "6="   + CTPObject.OnRspAvgTime             +  "\001"; }
        if ( CTPObject.OnRspOrderRef            !=""  ) {  ConvertedString = ConvertedString + "11="  + CTPObject.OnRspOrderRef            +  "\001"; }
        if ( CTPObject.OnRspVolumeTraded        !=""  ) {  ConvertedString = ConvertedString + "14="  + CTPObject.OnRspVolumeTraded        +  "\001"; }
        if ( CTPObject.OnRspLastPartiallyFilled !=""  ) {  ConvertedString = ConvertedString + "32="  + CTPObject.OnRspLastPartiallyFilled +  "\001"; }
        if ( CTPObject.OnRspRequestID           !=""  ) {  ConvertedString = ConvertedString + "37="  + CTPObject.OnRspRequestID           +  "\001"; }
        if ( CTPObject.OnRspVolumeTotalOriginal !=""  ) {  ConvertedString = ConvertedString + "38="  + CTPObject.OnRspVolumeTotalOriginal +  "\001"; }
        if ( CTPObject.OnRspOrderStatusType     !=""  ) {  ConvertedString = ConvertedString + "39="  + CTPObject.OnRspOrderStatusType     +  "\001"; }
        if ( CTPObject.OnRspOrderType           !=""  ) {  ConvertedString = ConvertedString + "54="  + CTPObject.OnRspOrderType           +  "\001"; }
        if ( CTPObject.OnRspCUSIP               !=""  ) {  ConvertedString = ConvertedString + "48="  + CTPObject.OnRspCUSIP               +  "\001"; }
        if ( CTPObject.OnRspExecutionTime       !=""  ) {  ConvertedString = ConvertedString + "60="  + CTPObject.OnRspExecutionTime       +  "\001"; }
        if ( CTPObject.OnRspMarketOrderPrice    !=""  ) {  ConvertedString = ConvertedString + "44="  + CTPObject.OnRspMarketOrderPrice    +  "\001"; }
        if ( CTPObject.OnRspPendingQuantity     !=""  ) {  ConvertedString = ConvertedString + "151=" + CTPObject.OnRspPendingQuantity     +  "\001"; } 
        if ( CTPObject.OnRspExchange            !=""  ) {  ConvertedString = ConvertedString + "207=" + CTPObject.OnRspExchange            +  "\001"; }

    }
    else{
    /*-------------------------------------------------------------For Market----------------------------------------------------------------------------------------------------------------------------------*/
    if( CTPObject.BidVolume5!="" && CTPObject.BidPrice5!=""){
        ConvertedString="";
        ConvertedString = ConvertedString + "8="   + "FIX.4.2"    +  "\001";                 
        ConvertedString = ConvertedString + "9="   + "130"        +  "\001"; 
        ConvertedString = ConvertedString + "35="   + "8"        +  "\001"; 
        ConvertedString = ConvertedString + "269=0\001" + "290=5\001" + "270=" + CTPObject.BidPrice5 + "\001" + "271=" + CTPObject.BidVolume5 + "\001"; 
        ConvertedString = ConvertedString + "10="  + "123"       +  "\001"; 
        QUEUE_FIX_RECEIVED.push(ConvertedString);
    
    }
    if( CTPObject.BidVolume4!="" && CTPObject.BidPrice4!=""){
        ConvertedString="";
        ConvertedString = ConvertedString + "8="   + "FIX.4.2"    +  "\001";                 
        ConvertedString = ConvertedString + "9="   + "130"        +  "\001"; 
        ConvertedString = ConvertedString + "35="   + "8"        +  "\001"; 
        ConvertedString = ConvertedString + "269=0\001" + "290=4\001" + "270=" + CTPObject.BidPrice4 + "\001" + "271=" + CTPObject.BidVolume4 + "\001"; 
        ConvertedString = ConvertedString + "10="  + "123"       +  "\001";            
            QUEUE_FIX_RECEIVED.push(ConvertedString);
    }
    if( CTPObject.BidVolume3!="" && CTPObject.BidPrice3!=""){
        ConvertedString="";
        ConvertedString = ConvertedString + "8="   + "FIX.4.2"    +  "\001";                 
        ConvertedString = ConvertedString + "9="   + "130"        +  "\001"; 
        ConvertedString = ConvertedString + "35="   + "8"        +  "\001"; 
        ConvertedString = ConvertedString + "269=0\001" + "290=3\001" + "270=" + CTPObject.BidPrice3 + "\001" + "271=" + CTPObject.BidVolume3 + "\001"; 
        ConvertedString = ConvertedString + "10="  + "123"       +  "\001";            
            QUEUE_FIX_RECEIVED.push(ConvertedString);
    }
    if( CTPObject.BidVolume2!="" && CTPObject.BidPrice2!=""){
        ConvertedString="";
        ConvertedString = ConvertedString + "8="   + "FIX.4.2"    +  "\001";                 
        ConvertedString = ConvertedString + "9="   + "130"        +  "\001"; 
        ConvertedString = ConvertedString + "35="   + "8"        +  "\001"; 
        ConvertedString = ConvertedString + "269=0\001" + "290=2\001" + "270=" + CTPObject.BidPrice2 + "\001" + "271=" + CTPObject.BidVolume2 + "\001"; 
        ConvertedString = ConvertedString + "10="  + "123"       +  "\001";            
            QUEUE_FIX_RECEIVED.push(ConvertedString);
    }
    if( CTPObject.BidVolume1!="" && CTPObject.BidPrice1!=""){
        ConvertedString="";
        ConvertedString = ConvertedString + "8="   + "FIX.4.2"    +  "\001";                 
        ConvertedString = ConvertedString + "9="   + "130"        +  "\001"; 
        ConvertedString = ConvertedString + "35="   + "8"        +  "\001"; 
        ConvertedString = ConvertedString + "269=0\001" + "290=1\001" + "270=" + CTPObject.BidPrice1 + "\001" + "271=" + CTPObject.BidVolume1 + "\001"; 
        ConvertedString = ConvertedString + "10="  + "123"       +  "\001";            
            QUEUE_FIX_RECEIVED.push(ConvertedString);
    }
    if( CTPObject.AskVolume5!="" && CTPObject.AskPrice5!=""){
        ConvertedString="";
        ConvertedString = ConvertedString + "8="   + "FIX.4.2"    +  "\001";                 
        ConvertedString = ConvertedString + "9="   + "130"        +  "\001"; 
        ConvertedString = ConvertedString + "35="   + "8"        +  "\001"; 
        ConvertedString = ConvertedString + "269=1\001" + "290=5\001" + "270=" + CTPObject.AskPrice5 + "\001" + "271=" + CTPObject.AskVolume5 + "\001"; 
        ConvertedString = ConvertedString + "10="  + "123"       +  "\001";            
        QUEUE_FIX_RECEIVED.push(ConvertedString);
    }
    if( CTPObject.AskVolume4!="" && CTPObject.AskPrice4!=""){
        ConvertedString="";
        ConvertedString = ConvertedString + "8="   + "FIX.4.2"    +  "\001";                 
        ConvertedString = ConvertedString + "9="   + "130"        +  "\001"; 
        ConvertedString = ConvertedString + "35="   + "8"        +  "\001"; 
        ConvertedString = ConvertedString + "269=1\001" + "290=4\001" + "270=" + CTPObject.AskPrice4 + "\001" + "271=" + CTPObject.AskVolume4 + "\001"; 
        ConvertedString = ConvertedString + "10="  + "123"       +  "\001";            
        QUEUE_FIX_RECEIVED.push(ConvertedString);
    }
    if( CTPObject.AskVolume3!="" && CTPObject.AskPrice3!=""){
        ConvertedString="";
        ConvertedString = ConvertedString + "8="   + "FIX.4.2"    +  "\001";                 
        ConvertedString = ConvertedString + "9="   + "130"        +  "\001"; 
        ConvertedString = ConvertedString + "35="   + "8"        +  "\001"; 
        ConvertedString = ConvertedString + "269=1\001" + "290=3\001" + "270=" + CTPObject.AskPrice3 + "\001" + "271=" + CTPObject.AskVolume3 + "\001"; 
        ConvertedString = ConvertedString + "10="  + "123"       +  "\001";            
        QUEUE_FIX_RECEIVED.push(ConvertedString);
    }
    if( CTPObject.AskVolume2!="" && CTPObject.AskPrice2!=""){
        ConvertedString="";
        ConvertedString = ConvertedString + "8="   + "FIX.4.2"    +  "\001";                 
        ConvertedString = ConvertedString + "9="   + "130"        +  "\001"; 
        ConvertedString = ConvertedString + "35="   + "8"        +  "\001"; 
        ConvertedString = ConvertedString + "269=1\001" + "290=2\001" + "270=" + CTPObject.AskPrice2 + "\001" + "271=" + CTPObject.AskVolume2 + "\001"; 
        ConvertedString = ConvertedString + "10="  + "123"       +  "\001";            
        QUEUE_FIX_RECEIVED.push(ConvertedString);
    }
    if( CTPObject.AskVolume1!="" && CTPObject.AskPrice1!=""){
        ConvertedString="";
        ConvertedString = ConvertedString + "8="   + "FIX.4.2"    +  "\001";                 
        ConvertedString = ConvertedString + "9="   + "130"        +  "\001"; 
        ConvertedString = ConvertedString + "35="   + "8"        +  "\001"; 
        ConvertedString = ConvertedString + "269=1\001" + "290=1\001" + "270=" + CTPObject.AskPrice1 + "\001" + "271=" + CTPObject.AskVolume1 + "\001"; 

    }
    }
    /*------------------------------------------------------------End For Market --------------------------------------------------------------------------------------------------------------------------------*/
    /*  
        if ( CTPObject.ReqBeginString    !=""  )      {    ConvertedString = ConvertedString + "8="   + CTPObject.ReqBeginString    +  "\001"; }                
        if ( CTPObject.ReqLength         !=""  )      {    ConvertedString = ConvertedString + "9="   + CTPObject.ReqLength         +  "\001"; }              
        if ( CTPObject.ReqOrderReqType   !=""  )      {    ConvertedString = ConvertedString + "35="  + CTPObject.ReqOrderReqType   +  "\001"; }             
        if ( CTPObject.ReqRequestID      !=""  )      {    ConvertedString = ConvertedString + "34="  + CTPObject.ReqRequestID      +  "\001"; }                
        if ( CTPObject.ReqSenderCompID   !=""  )      {    ConvertedString = ConvertedString + "49="  + CTPObject.ReqSenderCompID   +  "\001"; }             
        if ( CTPObject.ReqSendingTime    !=""  )      {    ConvertedString = ConvertedString + "52="  + CTPObject.ReqSendingTime    +  "\001"; }              
        if ( CTPObject.ReqTargetCompID   !=""  )      {    ConvertedString = ConvertedString + "56="  + CTPObject.ReqTargetCompID   +  "\001"; }             
        if ( CTPObject.ReqAccountID      !=""  )      {    ConvertedString = ConvertedString + "1="   + CTPObject.ReqAccountID      +  "\001"; }               
        if ( CTPObject.ReqOrderRef       !=""  )      {    ConvertedString = ConvertedString + "11="  + CTPObject.ReqOrderRef       +  "\001"; }             
        if ( CTPObject.ReqConst          !=""  )      {    ConvertedString = ConvertedString + "21="  + CTPObject.ReqConst          +  "\001"; }                
        if ( CTPObject.ReqLimitPrice     !=""  )      {    ConvertedString = ConvertedString + "44="  + CTPObject.ReqLimitPrice     +  "\001"; }               
        if ( CTPObject.ReqOrderPriceType !=""  )      {    ConvertedString = ConvertedString + "40="  + CTPObject.ReqOrderPriceType +  "\001"; }               
        if ( CTPObject.ReqOrderQty       !=""  )      {    ConvertedString = ConvertedString + "38="  + CTPObject.ReqOrderQty       +  "\001"; }             
        if ( CTPObject.ReqDirection      !=""  )      {    ConvertedString = ConvertedString + "54="  + CTPObject.ReqDirection      +  "\001"; }                
        if ( CTPObject.ReqInstrumentID   !=""  )      {    ConvertedString = ConvertedString + "55="  + CTPObject.ReqInstrumentID   +  "\001"; }             
        if ( CTPObject.ReqTransactTime   !=""  )      {    ConvertedString = ConvertedString + "60="  + CTPObject.ReqTransactTime   +  "\001"; }             
        if ( CTPObject.ReqFUT            !=""  )      {    ConvertedString = ConvertedString + "167=" + CTPObject.ReqFUT            +  "\001"; }               
        if ( CTPObject.ReqExpiryDate     !=""  )      {    ConvertedString = ConvertedString + "200=" + CTPObject.ReqExpiryDate     +  "\001"; }                
        if ( CTPObject.ReqExchangeID       !=""  )      {    ConvertedString = ConvertedString + "207=" + CTPObject.ReqExchangeID       +  "\001"; }              
        if ( CTPObject.ReqChecksum       !=""  )      {    ConvertedString = ConvertedString + "10="  + CTPObject.ReqChecksum       +  "\001"; } 
    */
    ConvertedString = ConvertedString + "10="  + "123"       +  "\001";            

    return ConvertedString;
}

void StringSparse::convertToObj(int tagNum,string message)
{
        if      ( tagNum== 8)    ConvertedObj.ReqBeginString =message;
        else if ( tagNum== 9)    ConvertedObj.ReqLength =message;
        else if ( tagNum== 35)   ConvertedObj.ReqOrderReqType =message;
        else if ( tagNum== 34)   ConvertedObj.ReqRequestID =message;
        else if ( tagNum== 49)   ConvertedObj.ReqSenderCompID =message;
        else if ( tagNum== 52)   ConvertedObj.ReqSendingTime =message;
        else if ( tagNum== 56)   ConvertedObj.ReqTargetCompID =message;
        else if ( tagNum== 1)    ConvertedObj.ReqAccountID =message;
        else if ( tagNum== 11)   ConvertedObj.ReqOrderRef =message;
        else if ( tagNum== 21)   ConvertedObj.ReqConst =message;
        else if ( tagNum== 44)   ConvertedObj.ReqLimitPrice =message;
        else if ( tagNum== 40)   ConvertedObj.ReqOrderPriceType =message;
        else if ( tagNum== 38)   ConvertedObj.ReqOrderQty =message;
        else if ( tagNum== 54)   ConvertedObj.ReqDirection =message;
        else if ( tagNum== 55)   ConvertedObj.ReqInstrumentID =message;
        else if ( tagNum== 60)   ConvertedObj.ReqTransactTime =message;
        else if ( tagNum== 167)  ConvertedObj.ReqFUT =message;
        else if ( tagNum== 200)  ConvertedObj.ReqExpiryDate =message;
        else if ( tagNum== 207)  ConvertedObj.ReqExchangeID =message;
        else if ( tagNum== 10)   ConvertedObj.ReqChecksum =message;
}





/*-----------------------------------------------------------------For Market Request----------------------------------------------------------------*/



#include <math.h>
#ifdef WINDOWS
#include <condition_variable>
#include <mutex>
std::mutex mtx;
std::condition_variable cv;
std::unique_lock<std::mutex> lck(mtx);
bool waitForData = false;
#else
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#endif
#include "code_convert.h"


std::string Convert (double number){
    std::ostringstream buf;
    buf << number;
    std::string doule_to_string(buf.str()),final="";
    int count=0; // No need to call final.length().
    for(int i=0;i<doule_to_string.length();i++)
    {
        if(doule_to_string[i]!='.') // There is a chance of getting this case.
        {
            count++;
            final+=doule_to_string[i];
        }
        if(count==6) // Only starting 6 integers are required.
            break;
    }
    //std::cout<< final<<std::endl;
    return final;
}


bool waitForData = false;

int requestId=0;
char input[100];

CThostFtdcReqUserLoginField userLoginField;
CThostFtdcUserLogoutField userLogoutField;
CThostFtdcQrySettlementInfoField pQryOrder;
sem_t sem;
void Print_Login_Details(){
    printf("********UserLoginField***********\n");
    printf("BrokerID\t:\t%s\n",     userLoginField.BrokerID);
    printf("UserID\t\t:\t%s\n",     userLoginField.UserID);
    printf("Password\t:\t%s\n",     userLoginField.Password);
    printf("UserProductInfo\t:\t%s\n",      userLoginField.UserProductInfo);
    printf("InterfaceProductInfo\t:\t%s\n",     userLoginField.InterfaceProductInfo);
    printf("ProtocolInfo\t:\t%s\n",     userLoginField.ProtocolInfo);
    printf("MacAddress\t:\t%s\n",   userLoginField.MacAddress);
    printf("OneTimePassword\t:\t%s\n",      userLoginField.OneTimePassword);
    printf("ClientIPAddress\t:\t%s\n",      userLoginField.ClientIPAddress);
        //strcpy(userLoginField.TradingDay,"Wednesday");
    printf("TradingDay\t:\t%s\n",   userLoginField.TradingDay);
}

void Print_Logout_Details(){
    printf("userLogoutField.UserID:- %s   userLogoutField.BrokerID:- %s\n",userLogoutField.UserID,userLogoutField.BrokerID );
}


// class CTraderHandler : public CThostFtdcTraderSpi{
class CTraderHandler : public CThostFtdcMdSpi{

    public:

    CTraderHandler(){
        // printf("CTraderHandler:called.\n");
    }

    virtual void OnFrontConnected() {
        static int i = 0;
        if (i++==0) {
            printf("OnFrontConnected:called.\n");
            #ifdef WINDOWS
            cv.notify_one();
            #else
            sem_post(&sem);
            #endif
        }
    }


    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
        printf("OnRspUserLogin:called\n");
        if (pRspInfo->ErrorID == 0) {
            printf("Login successful!!!!!!\n");         
            printf("nRequestID: %d\n", nRequestID);
            printf("requestId: %d\n", requestId);
            #ifdef WINDOWS
            cv.notify_one();
            #else
            sem_post(&sem);
            #endif
        }else{
            printf("Login failed ErrorID[%d]\n",pRspInfo->ErrorID);
            #ifdef WINDOWS
            cv.notify_one();
            #else
            sem_post(&sem);
            #endif
        }
    }


    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,  CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) { 
        printf("OnReqUserLogout:called\n");

        printf("nRequestID: %d\n", nRequestID);
        printf("requestId: %d\n", requestId);
        
        if (pRspInfo->ErrorID == 0) {
            printf("Log out successfully!\n");
            // exit(0);
            #ifdef WINDOWS
            cv.notify_one();
            #else
            sem_post(&sem);
            #endif
        }else{
            printf("Logout failed! with ErrorID[%d]\n",pRspInfo->ErrorID);
            #ifdef WINDOWS
            cv.notify_one();
            #else
            sem_post(&sem);
            #endif
        }
    }
    
    virtual void OnRspError (CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
    {
        char ErrorMsg[243];
        printf("OnRspError():Is executed... nRequestID:- %d and ErrorID=[%d]\n",nRequestID,pRspInfo->ErrorID);
        // printf("%s\n", pRspInfo->ErrorMsg);
        gbk2utf8(pRspInfo->ErrorMsg,ErrorMsg,sizeof(ErrorMsg));
        #ifdef WINDOWS
        cv.notify_one();
        #else
        sem_post(&sem);
        #endif
    }

    virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
    {
        #ifdef WINDOWS
        cv.notify_one();
        #else
        sem_post(&sem);
        #endif
        waitForData = true;
    }

    ///È¡Ïû¶©ÔÄÐÐÇéÓ¦´ð
    virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
    {
        #ifdef WINDOWS
        cv.notify_one();
        #else
        sem_post(&sem);
        #endif
    }

    ///¶©ÔÄÑ¯¼ÛÓ¦´ð
    virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
    {
        #ifdef WINDOWS
        cv.notify_one();
        #else
        sem_post(&sem);
        #endif
    }

    ///È¡Ïû¶©ÔÄÑ¯¼ÛÓ¦´ð
    virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
    {
        #ifdef WINDOWS
        cv.notify_one();
        #else
        sem_post(&sem);
        #endif
    }

    ///Éî¶ÈÐÐÇéÍ¨Öª
    virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) 
    {
        printf("==================================================================================================================\n");
        printf("Markert data\n");
        printf("ActionDay %s\n", pDepthMarketData->ActionDay);
        printf("AskPrice1 = %lf\n", pDepthMarketData->AskPrice1);
        printf("AskPrice2 = %lf\n", pDepthMarketData->AskPrice2);
        printf("AskPrice3 = %lf\n", pDepthMarketData->AskPrice3);
        printf("%lf\n", pDepthMarketData->AskPrice4);
        printf("%lf\n", pDepthMarketData->AskPrice5);
        printf("%d\n", pDepthMarketData->AskVolume1);
        printf("%d\n", pDepthMarketData->AskVolume2);
        printf("%d\n", pDepthMarketData->AskVolume3);
        printf("%d\n", pDepthMarketData->AskVolume4);
        printf("%d\n", pDepthMarketData->AskVolume5);
        printf("%lf\n", pDepthMarketData->AveragePrice);
        printf("%lf\n", pDepthMarketData->BidPrice1);
        printf("%lf\n", pDepthMarketData->BidPrice2);
        printf("%lf\n", pDepthMarketData->BidPrice3);
        printf("%lf\n", pDepthMarketData->BidPrice4);
        printf("%lf\n", pDepthMarketData->BidPrice5);
        printf("%d\n", pDepthMarketData->BidVolume1);
        printf("%d\n", pDepthMarketData->BidVolume2);
        printf("%d\n", pDepthMarketData->BidVolume3);
        printf("%d\n", pDepthMarketData->BidVolume4);
        printf("%d\n", pDepthMarketData->BidVolume5);
        printf("%lf\n", pDepthMarketData->ClosePrice);
        printf("%lf\n", pDepthMarketData->CurrDelta);
        printf("%s\n", pDepthMarketData->ExchangeID);
        printf("HighestPrice = %lf\n", pDepthMarketData->HighestPrice);
        

        CTPStructure RspObject;

        RspObject.AskVolume1 = Convert( pDepthMarketData->AskVolume1 );
        RspObject.AskVolume2 = Convert( pDepthMarketData->AskVolume2 );
        RspObject.AskVolume3 = Convert( pDepthMarketData->AskVolume3 );
        RspObject.AskVolume4 = Convert( pDepthMarketData->AskVolume4 );
        RspObject.AskVolume5 = Convert( pDepthMarketData->AskVolume5 );


        RspObject.AskPrice1 = Convert( pDepthMarketData->AskPrice1 );
        RspObject.AskPrice2 = Convert( pDepthMarketData->AskPrice2 );
        RspObject.AskPrice3 = Convert( pDepthMarketData->AskPrice3 );
        RspObject.AskPrice4 = Convert( pDepthMarketData->AskPrice4 );
        RspObject.AskPrice5 = Convert( pDepthMarketData->AskPrice5 );


        RspObject.BidVolume1 = Convert( pDepthMarketData->BidVolume1 );
        RspObject.BidVolume2 = Convert( pDepthMarketData->BidVolume2 );
        RspObject.BidVolume3 = Convert( pDepthMarketData->BidVolume3 );
        RspObject.BidVolume4 = Convert( pDepthMarketData->BidVolume4 );
        RspObject.BidVolume5 = Convert( pDepthMarketData->BidVolume5 );


        RspObject.BidPrice1 = Convert( pDepthMarketData->BidPrice1 );
        RspObject.BidPrice2 = Convert( pDepthMarketData->BidPrice2 );
        RspObject.BidPrice3 = Convert( pDepthMarketData->BidPrice3 );
        RspObject.BidPrice4 = Convert( pDepthMarketData->BidPrice4 );
        RspObject.BidPrice5 = Convert( pDepthMarketData->BidPrice5 );

        pthread_mutex_lock( &Mutex_Recv );
        QUEUE_CTPObj_RECEIVED.push(RspObject);
        pthread_cond_signal(&condition_recv);               
        pthread_mutex_unlock( &Mutex_Recv );


        if(pDepthMarketData->BidVolume5 && pDepthMarketData->BidPrice5 && pDepthMarketData->AskVolume5 && pDepthMarketData->AskPrice5){
            waitForData = true;

            sem_post(&sem); 
        }
        printf("==================================================================================================================\n"); 
/*--------------------------------------------------------------------Converting to FIX String-----------------------------------------------*/
/*      std::stringstream ss;
        ss << std::fixed << number;
        std::string mystring = ss.str();
        cout << test <<endl;
        8=FIX.4.2|9=123|35=8|269=0|290=1|270=Starting_6|271=BidVolume1|10=
        convert="8=FIX.4.2|" + "9=123|" + "35=8|" + "269=0|" + "290=1|" + "270= " */
/*------------------------------------------------------------------------------END-------------------------------------------------------------*/
        // printf("%lf\n", pDepthMarketData->);
    }

    ///Ñ¯¼ÛÍ¨Öª
    virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) 
    {
        #ifdef WINDOWS
        cv.notify_one();
        #else
        sem_post(&sem);
        #endif
    }

    virtual void OnRspQryDepthMarketData( CThostFtdcDepthMarketDataField * pDepthMarketData, CThostFtdcRspInfoField * pRspInfo, int nRequestID, bool bIsLast) 
    {        
        if ( (pRspInfo) && (pRspInfo->ErrorID != 0) )  {
            char ErrorMsg[243];
            gbk2utf8(pRspInfo->ErrorMsg,ErrorMsg,sizeof(ErrorMsg));
            printf("OnRspQryDepthMarketData():An error occurred:ErrorId=%d,ErrorMsg=%s\n",pRspInfo->ErrorID,ErrorMsg);
            #ifdef WINDOWS
            cv.notify_one();
            #else
            sem_post(&sem);
            #endif
        }

        if ( pDepthMarketData != NULL ) 
        {
            char TradingDay[27];
            gbk2utf8(pDepthMarketData->TradingDay,TradingDay,sizeof(TradingDay));
            char InstrumentID[93];
            gbk2utf8(pDepthMarketData->InstrumentID,InstrumentID,sizeof(InstrumentID));
            char ExchangeID[27];
            gbk2utf8(pDepthMarketData->ExchangeID,ExchangeID,sizeof(ExchangeID));
            char ExchangeInstID[93];
            gbk2utf8(pDepthMarketData->ExchangeInstID,ExchangeInstID,sizeof(ExchangeInstID));
            double LastPrice = pDepthMarketData->LastPrice;
            double PreSettlementPrice = pDepthMarketData->PreSettlementPrice;
            double PreClosePrice = pDepthMarketData->PreClosePrice;
            double PreOpenInterest = pDepthMarketData->PreOpenInterest;
            double OpenPrice = pDepthMarketData->OpenPrice;
            double HighestPrice = pDepthMarketData->HighestPrice;
            double LowestPrice = pDepthMarketData->LowestPrice;
            int Volume = pDepthMarketData->Volume;
            double Turnover = pDepthMarketData->Turnover;
            double OpenInterest = pDepthMarketData->OpenInterest;
            double ClosePrice = pDepthMarketData->ClosePrice;
            double SettlementPrice = pDepthMarketData->SettlementPrice;
            double UpperLimitPrice = pDepthMarketData->UpperLimitPrice;
            double LowerLimitPrice = pDepthMarketData->LowerLimitPrice;
            double PreDelta = pDepthMarketData->PreDelta;
            double CurrDelta = pDepthMarketData->CurrDelta;
            char UpdateTime[27];
            gbk2utf8(pDepthMarketData->UpdateTime,UpdateTime,sizeof(UpdateTime));
            int UpdateMillisec = pDepthMarketData->UpdateMillisec;
            double BidPrice1 = pDepthMarketData->BidPrice1;
            int BidVolume1 = pDepthMarketData->BidVolume1;
            double AskPrice1 = pDepthMarketData->AskPrice1;
            int AskVolume1 = pDepthMarketData->AskVolume1;
            double BidPrice2 = pDepthMarketData->BidPrice2;
            int BidVolume2 = pDepthMarketData->BidVolume2;
            double AskPrice2 = pDepthMarketData->AskPrice2;
            int AskVolume2 = pDepthMarketData->AskVolume2;
            double BidPrice3 = pDepthMarketData->BidPrice3;
            int BidVolume3 = pDepthMarketData->BidVolume3;
            double AskPrice3 = pDepthMarketData->AskPrice3;
            int AskVolume3 = pDepthMarketData->AskVolume3;
            double BidPrice4 = pDepthMarketData->BidPrice4;
            int BidVolume4 = pDepthMarketData->BidVolume4;
            double AskPrice4 = pDepthMarketData->AskPrice4;
            int AskVolume4 = pDepthMarketData->AskVolume4;
            double BidPrice5 = pDepthMarketData->BidPrice5;
            int BidVolume5 = pDepthMarketData->BidVolume5;
            double AskPrice5 = pDepthMarketData->AskPrice5;
            int AskVolume5 = pDepthMarketData->AskVolume5;
            double AveragePrice = pDepthMarketData->AveragePrice;
            char ActionDay[27];
            gbk2utf8(pDepthMarketData->ActionDay,ActionDay,sizeof(ActionDay));

            printf("TradingDay=%s,",TradingDay);    
            printf("InstrumentID=%s,",InstrumentID);  
            printf("LastPrice=%f\n",LastPrice);  
            #ifdef WINDOWS
            cv.notify_one();
            #else
            sem_post(&sem);
            #endif
        }

        if(bIsLast) {
            #ifdef WINDOWS
            cv.notify_one();
            #else
            sem_post(&sem);
            #endif
        }
    }
/*        virtual void OnRspSettlementInfoConfirm( CThostFtdcSettlementInfoConfirmField * pSettlementInfoConfirm, CThostFtdcRspInfoField * pRspInfo, int nRequestID, bool bIsLast) 
        {
        printf("OnRspSettlementInfoConfirm():Executed...\n");

        
        if ( (pRspInfo) && (pRspInfo->ErrorID != 0) )  {
            

            char ErrorMsg[243];
            gbk2utf8(pRspInfo->ErrorMsg,ErrorMsg,sizeof(ErrorMsg));
            printf("OnRspSettlementInfoConfirm(): ErrorId=%d,ErrorMsg=%s\n",pRspInfo->ErrorID,ErrorMsg);
        }

        
        if ( pSettlementInfoConfirm != NULL ) {
            

            char BrokerID[33];
            gbk2utf8(pSettlementInfoConfirm->BrokerID,BrokerID,sizeof(BrokerID));
            
            char InvestorID[39];
            gbk2utf8(pSettlementInfoConfirm->InvestorID,InvestorID,sizeof(InvestorID));
            
            char ConfirmDate[27];
            gbk2utf8(pSettlementInfoConfirm->ConfirmDate,ConfirmDate,sizeof(ConfirmDate));
            
            char ConfirmTime[27];
            gbk2utf8(pSettlementInfoConfirm->ConfirmTime,ConfirmTime,sizeof(ConfirmTime));
            
            printf("ConfirmDate=%s,ConfirmTime=%s\n",ConfirmDate,ConfirmTime);

        }


        if(bIsLast) {

            sem_post(&sem);
        }
    }*/
};


int Market()
{
    /************Reading from Config File************/
     //Read_from_Config_File();
    /***********************************************/

/*    CThostFtdcSettlementInfoConfirmField requestData;
    memset(&requestData,0,sizeof(requestData));
    strcpy(requestData.BrokerID,BrokerID);
    strcpy(requestData.InvestorID,UserID);
    strcpy(requestData.ConfirmDate,"");
    strcpy(requestData.ConfirmTime,"");
    int result = pUserApi->ReqSettlementInfoConfirm(&requestData,requestID++);

    printf("return Value of ReqSettlementInfoConfirm is %d\n",result);*/

    #ifndef WINDOWS 
    sem_init(&sem,0,0);
    #endif
    Print_Login_Details();  
    // /*------------------------Step 1 Event Handler------------------------*/
    CThostFtdcMdApi *pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
    CTraderHandler traderHandler = CTraderHandler();
    CTraderHandler * pTraderHandler = &traderHandler;

    pUserApi->RegisterSpi(pTraderHandler); 

    // /*------------------------Step 2 Register Front Send IP and Port------------------------*/
    pUserApi->RegisterFront(IPAddress);
    
    // /*------------------------Establish the connection between Member System and Trading System (It launch a thread) ------------------------*/
    pUserApi->Init();
    printf("I am in\n");
    #ifdef WINDOWS
    cv.wait(lck);
    #else
    sem_wait(&sem);
    #endif

    // /*------------------------User login details------------------------*/

    #ifdef WINDOWS 
        strcpy_s(userLoginField.BrokerID,BrokerID);
        strcpy_s(userLoginField.UserID,UserID);
        strcpy_s(userLoginField.Password,Password);
    #else
        strcpy(userLoginField.BrokerID,BrokerID);
        strcpy(userLoginField.UserID,UserID);
        strcpy(userLoginField.Password,Password);
    #endif
    
    pUserApi->ReqUserLogin(&userLoginField, 0);

    #ifdef WINDOWS
    cv.wait(lck);
    #else
    sem_wait(&sem);
    #endif
    while(1){
        if(QUEUE_CTPObj_SEND.size()>0)
        {
            CTPStructure ReqObject;
            ReqObject = QUEUE_CTPObj_SEND.front();
            cout << "Merket InstrumentID is " << ReqObject.ReqInstrumentID.c_str() <<endl;
            char *instrumentData ={"au1706"};//= (char *)ReqObject.ReqInstrumentID.c_str();////[50];
            // char *instrumentData = (char *)ReqObject.ReqInstrumentID.c_str();////[50];
            printf("%s\n", instrumentData);
            //strcpy(instrumentData, ReqObject.ReqInstrumentID.c_str());
            int returnVal = pUserApi->SubscribeMarketData(&instrumentData, 1);
            printf("Subscription result: %d\n", returnVal);

            #ifdef WINDOWS
            cv.wait(lck);
            #else
            sem_wait(&sem);
            #endif
            while(!waitForData)
            {
                if (waitForData == true)
                    break;
            }
            #ifdef WINDOWS
            cv.wait(lck);
            #else
            sem_wait(&sem);
            #endif
            QUEUE_CTPObj_SEND.pop();
        }
    else{
            pthread_cond_wait( &condition_Market_start, &Mutex_Market);
        }
    }
    return 0;
}




/*-----------------------------------------------------------------End For Market Request----------------------------------------------------------------*/
















/*********************************/
void *InitialThread(void *dummyPtr);

void* TCP_server_to_recv(void *dummyPtr);
void* TCP_server_to_send(void *dummyPtr);
void* UDP_server(void *dummyPtr); 

void *MiddleThread(void *dummyPtr);
void *FinalThread(void *dummyPtr);

void *THREAD_SEND(void *dummyPtr);
void *THREAD_RECEIVED(void *dummyPtr);

int main(int argc, char const *argv[])
{
    Read_from_Config_File();
    printf("I am in Main()\n");
    pthread_t thread1,thread2,thread3;
    pthread_create( &thread1, NULL, InitialThread, NULL );
    pthread_create( &thread2, NULL, MiddleThread, NULL );
    pthread_create( &thread3, NULL, FinalThread, NULL );
    pthread_join( thread1, NULL);   
    pthread_join( thread2, NULL);
    pthread_join( thread3, NULL);
    return 0;
}


void *InitialThread(void *dummyPtr)
{
    printf("I am Here\n");
    pthread_t thread1, thread2;
    
    int threadId1 = pthread_create( &thread1, NULL, TCP_server_to_recv, NULL);
    if(threadId1)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",threadId1);
         return 0;
     }
     int threadId2;
     if(!TCP_or_UDP)
        threadId2 = pthread_create( &thread1, NULL, TCP_server_to_send, NULL);
     else
        threadId2 = pthread_create( &thread1, NULL, UDP_server, NULL);
     if(threadId2)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",threadId2);
         return 0;
     }


     pthread_join( thread1, NULL);
     pthread_join( thread2, NULL); 

}

int TCP_recv_Client_socket;
void reconnect(){
    TCP_recv_Client_socket = accept(TCP_server_socket, (struct sockaddr *) &TCP_recv_serverStorage, &TCP_recv_addr_size);
    printf("Connected with the Client!!!!!\n");
}
#define response_size 1024
int TCP_send_Client_socket;
char response[response_size];
void reconnect_client(){
    TCP_send_Client_socket = accept(TCP_server_to_send_socket, (struct sockaddr *) &TCP_send_serverStorage, &TCP_send_addr_size);
    send(TCP_send_Client_socket,response, sizeof(response), 0);
    printf("Connected with the Client!!!!!\n");
}

void print_queue(){
    printf("------------------------------------------------------------------------------\n");
    while(!QUEUE_FIX_SEND.empty()){
        cout<<QUEUE_FIX_SEND.front()<<endl;
        QUEUE_FIX_SEND.pop();
    }
    printf("------------------------------------------------------------------------------\n");
}
void* TCP_server_to_recv(void *dummyPtr)
{
    char response[response_size];
    //int port=5000;
    /* Creating socket */
    TCP_server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(TCP_server_socket < 0){
        printf("Error in creating socket for TCP server recv!!!!\n");
        return 0;
    }
    /* End */

    /* Configure settings of the server address struct */
    TCP_recv_serverAddr.sin_family = AF_INET;
    TCP_recv_serverAddr.sin_port = htons(TCP_Port);
    TCP_recv_serverAddr.sin_addr.s_addr = inet_addr(TCP_IP_Address);
    /* End */

    memset(TCP_recv_serverAddr.sin_zero, '\0', sizeof(TCP_recv_serverAddr.sin_zero));  

    /* Bind the address struct to the socket */
    if(bind(TCP_server_socket, (struct sockaddr *) &TCP_recv_serverAddr, sizeof(TCP_recv_serverAddr)) < 0)
    {
        printf("Error in binding in TCP_server_to_recv!!!!!\n");
        return 0;
    }

    /* Listen on the socket, with 5 max connection requests queued */
    if(listen(TCP_server_socket,5)==0)
        printf("Listening\n");
    else
        printf("Error\n");
    TCP_recv_Client_socket = accept(TCP_server_socket, (struct sockaddr *) &TCP_recv_serverStorage, &TCP_recv_addr_size);
    /*Continuously receiving from the Client!!!!!!!*/
    while(1){
        int tmp = recv(TCP_recv_Client_socket,buffer, 1024, 0); 

        pthread_mutex_lock( &count_mutex );
        printf("Mutex Lock\n");
        
        QUEUE_FIX_SEND.push(buffer);
        //QUEUE_FIX_RECEIVED.push(buffer);
        pthread_cond_signal( &condition_print );
        
        printf("Mutex Unlock\n");
        pthread_mutex_unlock( &count_mutex );
        /* For Testing the queue*/
    /*
                if(strlen(buffer)==5){
                    print_queue();

            }*/
        if(tmp == 0){
            printf("Error in recv from Client 1 Expected a disconnect!!!!!\n");
            printf("Trying to reconnect..........\n");
            reconnect();
        }
        else
            printf("Message from Client->%s and the return value is %d\n", buffer, tmp);
    }
    /*---------------------End---------------------------*/
}

void* TCP_server_to_send(void *dummyPtr)
{
    
    //int port=5000;
    /* Creating socket */
    TCP_server_to_send_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(TCP_server_to_send_socket < 0){
        printf("Error in creating socket for TCP server send!!!!\n");
        return 0;
    }
    /* End */

    /* Configure settings of the server address struct */
    TCP_send_serverAddr.sin_family = AF_INET;
    TCP_send_serverAddr.sin_port = htons(TCP_UDP_Port);
    TCP_send_serverAddr.sin_addr.s_addr = inet_addr(TCP_UDP_IP_Adrress);
    /* End */

    memset(TCP_send_serverAddr.sin_zero, '\0', sizeof(TCP_send_serverAddr.sin_zero));  

    /* Bind the address struct to the socket */
    if(bind(TCP_server_to_send_socket, (struct sockaddr *) &TCP_send_serverAddr, sizeof(TCP_send_serverAddr)) < 0)
    {
        printf("Error in binding in TCP_server_to_send!!!!!\n");
        return 0;
    }

    /* Listen on the socket, with 5 max connection requests queued */
    if(listen(TCP_server_to_send_socket,5)==0)
        printf("Listening\n");
    else
        printf("Error\n");
    TCP_send_Client_socket = accept(TCP_server_to_send_socket, (struct sockaddr *) &TCP_send_serverStorage, &TCP_send_addr_size);
    /*Continuously receiving from the Client!!!!!!!*/
    while(1){
        bzero(response, response_size);
        printf("Enter the message to the client\n");
        if(!QUEUE_FIX_RECEIVED.empty()){
          strcpy(response,QUEUE_FIX_RECEIVED.front().c_str());
          printf("---------------------------The response message is %s and Clinet socket %d\n", response, TCP_send_Client_socket);
          int tmp = write(TCP_send_Client_socket,response, sizeof(response)); 
          printf("---------------------------The response message is %s\n", response);
          QUEUE_FIX_RECEIVED.pop();
          printf("N value %d\n", tmp);
          if (tmp <= 0) 
          {
            printf("Error in Sending to Clinet Reconnecting with the client!!!!!\n");
            reconnect_client();
          }
          pthread_mutex_unlock( &count_mutex );
        }
        else
        {
          pthread_mutex_lock( &count_mutex );
          printf("QUEUE_FIX_RECEIVED is empty!!!!!!! Waiting for some response.......\n");
          pthread_cond_wait( &condition_var, &count_mutex );
        }
    /*---------------------End---------------------------*/
}
}

void* UDP_server(void *dummyPtr) 
{
  int sockfd; /* socket */
  struct sockaddr_in serveraddr; /* server's addr */
  char buf[BUFSIZE]; /* message buf */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sockfd < 0) 
    printf("ERROR opening socket \n");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (const void *)&optval , sizeof(int));
  struct sockaddr_in Sender_addr;
  bzero((char *) &Sender_addr, sizeof(Sender_addr));
  Sender_addr.sin_family = AF_INET;
  Sender_addr.sin_port = htons(TCP_UDP_Port);
  inet_pton(AF_INET, TCP_UDP_IP_Adrress, &Sender_addr.sin_addr); 

  /*
   * build the server's Internet address
   */
  int len = sizeof(Sender_addr);
  while (1) {
    bzero(buf, BUFSIZE);
    //printf("Enter the message to the client\n");
    //scanf("%s",buf);
    if(!QUEUE_FIX_RECEIVED.empty()){
      char mess[1024];
      strcpy(mess,QUEUE_FIX_RECEIVED.front().c_str());
      printf("---------------------------The response message is %s\n", mess);
      n = sendto(sockfd, mess, sizeof(buf), 0,  (struct sockaddr *) &Sender_addr, len);
      printf("---------------------------The response message is %s\n", mess);
      QUEUE_FIX_RECEIVED.pop();
      printf("N value %d\n", n);
      if (n < 0) 
        printf("ERROR in sendto \n");
      pthread_mutex_unlock( &count_mutex );

    }
    else
    {
      pthread_mutex_lock( &count_mutex );
      printf("QUEUE_FIX_RECEIVED is empty!!!!!!! Waiting for some response.......\n");
      pthread_cond_wait( &condition_var, &count_mutex );
    }
  }
}

//////////////////////////////////Middle Thread//////////////////////////////////

void *MiddleThread(void *dummyPtr)
{
    pthread_t thread1,thread2;
    
    int threadId1 = pthread_create( &thread1, NULL, THREAD_SEND, NULL);
    if(threadId1)
    {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",threadId1);
         return 0;
    }

    int threadId2 = pthread_create( &thread1, NULL, UDP_server, NULL);
    if(threadId2)
    {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",threadId2);
         return 0;
    }
 
    pthread_join( thread1, NULL);   
    pthread_join( thread2, NULL);

}


void *THREAD_SEND(void *dummyPtr)
{
    /**************************Thread IN****************/
    printf("Thread for Sparsing Request Message Launched..!!\n");
    while(1)
    {
        if(QUEUE_FIX_SEND.size()>0)
        {
            FixINString=QUEUE_FIX_SEND.front().c_str();
            QUEUE_FIX_SEND.pop();

            
            ObjIN=ThreadIN.convertFromFixToCTPObj(FixINString);
            
            pthread_mutex_lock( &Mutex_Market );

            QUEUE_CTPObj_SEND.push(ObjIN);

            pthread_cond_signal( &condition_Market_start );
               
            pthread_mutex_unlock( &Mutex_Market );
        }
        else
        {
            pthread_cond_wait( &condition_print, &count_mutex ); /*Signalis from TCP_Recv*/
            printf("QUEUE_FIX_SEND is empty!!!!!!!\n");
        }
    }
    /**************************************************/

}
void *THREAD_RECEIVED(void *dummyPtr) /**/
{
    /**********Thread OUT***************/
    printf("Thread for Sparsing Response Object Launched..!!\n");
    while(1)
    {
        if(QUEUE_CTPObj_RECEIVED.size()>0)
        {
            ObjOUT=QUEUE_CTPObj_RECEIVED.front();
            QUEUE_CTPObj_RECEIVED.pop();
        
            FixOUTString=ThreadOUT.convertFromCTPObjToFix(ObjOUT);
            
            
            pthread_mutex_lock( &Mutex_UDP );
            
            QUEUE_FIX_RECEIVED.push(FixOUTString);
        
            pthread_cond_signal( &condition_var );
            pthread_mutex_unlock( &Mutex_UDP );
        }
        else
        {   
            pthread_cond_wait( &condition_recv, &Mutex_Recv); /*Signal is in Market response*/
            printf("QUEUE_FIX_RECEIVED is Empty..!!\n");
        }
    }
    /***********************************/
}
void *FinalThread(void *dummyPtr){
    if(!Market_or_Order)
    {
        printf("Requesting for Market Data!!!!\n");
        /*while(1)
        {*/
                Market();
        /*    else
                pthread_cond_signal( &condition_var );
        }*/
    }
    else{
        printf("Now this work only for Order!!!!\n");
    }
}
