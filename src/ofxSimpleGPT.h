/*-----------------------------------------------------------/
ofxSimpleGPT.h

github.com/azuremous
Created by Jung un Kim a.k.a azuremous on 10/2/24.
/----------------------------------------------------------*/
#pragma once

#include "ofxSimpleRestAPI.h"
#include "ofThread.h"
#include "ofJson.h"

class ofxSimpleGPT : ofThread{
private:
    ofThreadChannel<bool>request;
    
    ofxSimpleRestAPI restAPI;
    string key;
    string model;
    string endPoint;
    int maxTokens;
    int timeoutSec;
    
protected:
    void threadedFunction(){
        bool bang;
        while( isThreadRunning() ){
            if(request.receive(bang)){
                int status = restAPI.getResponseStatus();
                if (status == 200) {
                    ofJson resultData = ofJson::parse(restAPI.getData());
                    string result = resultData["choices"][0]["message"]["content"].get<string>();
                    ofNotifyEvent(message, result);
                    
                } else {
                    auto errorCode = restAPI.getError();
                    ofLogError("ofxSimpleGPT", errorCode);
                }
            }
        }
    }
public:
    ofEvent<string>message;
    
    ofxSimpleGPT():maxTokens(256),timeoutSec(60)
    {
        startThread();
    }
    
    ~ofxSimpleGPT(){
        request.close();
        stopThread();
    }
    
    void setup(const string &endPoint, const string &key, const string &model, bool showDetailLog = false){
        this->endPoint = endPoint;
        this->key = key;
        this->model = model;
        if(showDetailLog){
            restAPI.showDetailLog();
        }
    }
    
    void setCAPath(const string &path){
        restAPI.setCAPath(path);
    }
    
    void setMaxToken(const int &size){
        maxTokens = size;
    }
    
    void setTimeout(const int &sec){
        timeoutSec = sec;
    }
    
    void chat(const string &message){
        ofJson requestBody;
        requestBody["model"] = model;
        requestBody["messages"].push_back({{"role", "user"}, {"content", message}});
        requestBody["temperature"] = 1;
        requestBody["max_tokens"] = maxTokens;
        requestBody["top_p"] = 1;
        requestBody["frequency_penalty"] = 0;
        requestBody["presence_penalty"] = 0;
        
        restAPI.setRequest(endPoint, ofHttpRequest::Method::POST, timeoutSec, "application/json");
        restAPI.addHeader("Authorization", "Bearer "+key);
        restAPI.setRequestBody(requestBody.dump());
        
        request.send(true);
    }
};
