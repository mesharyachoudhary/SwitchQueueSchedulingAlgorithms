#include <bits/stdc++.h>
using namespace std;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
int packetId = 0;                   //  Denotes the id of a generated packet
int timer = 0;                      //  Denotes the current time slot
int numOfInputPorts = 8;            //  Number of input ports
int numOfOutputPorts = 8;           //  Number of output ports
int bufferSize = 4;                 //  Size of the input/output queues
double packetGenProb = 0.5;         //  Packet Generation Probability
string policy = "INQ";              //  Denotes the scheduling policy
int maxTimeSlots = 10000;           //  Number of time slots for which the program runs
int K = 5;                          //  Knockout
double kouqdropprob = 0;            //  Denotes the KOUQ Drop Probability
double standarddevpacketdelay = 0;  //  Denotes the standard Deviation Of Packet Delay
queue <int> inputPort[50];          //  Queues for the Input Ports
queue <int> outputPort[50];         //  Queues for the Output Ports
vector <int> contention[50];        //  Stores the packet ids of the packets contending for the ith Output Port
queue <int> VOQinputPort[50][50];   //  Stores virtual Output Queues for the ith Input Port
double link[50][50];                //  Counts the number of times link from ith Input Port to jth Output port was used
double linktemp[50][50];            //  A temporary data structure to store the count for a particular iteration
map <int,int> arrivalPort;          //  Stores the input port for a packet
map <int,int> destinationPort;      //  Stores the output port for a packet
map <int,int> arrivalTime;          //  Stores the arrival time for a packet
map <int,int> completionTime;       //  Stores the completion time for a packet
map <int,double> startTime;         //  Stores the start time for a packet

//  Generates traffic for all input ports with the packet generation probability
void GenerateTraffic(int scaledProb){
     for(int i=0;i<numOfInputPorts;i++){
         for(int j=0;j<numOfOutputPorts;j++){
              linktemp[i][j]=0;
         }
     }
     for(int i = 0; i < numOfInputPorts; i++){ 
         if(uniform_int_distribution <int> (0,999)(rng) < scaledProb){
             if(inputPort[i].size()<bufferSize){
             inputPort[i].push(packetId);
             destinationPort[packetId] = uniform_int_distribution <int> (0,numOfOutputPorts-1)(rng);
             arrivalPort[packetId]=i;
             arrivalTime[packetId] = timer;
             startTime[packetId] = (double)(timer) + uniform_real_distribution<>(0.001,0.01)(rng);
             }
             packetId++;
         }
     }
}

//  Places the packet in the output queue
void PlaceInOutputBuffer(int inputPortNumber){
    if(inputPort[inputPortNumber].size()>0){
    int packettotransmit = inputPort[inputPortNumber].front();
    inputPort[inputPortNumber].pop();
    if(outputPort[destinationPort[packettotransmit]].size()<bufferSize){
    outputPort[destinationPort[packettotransmit]].push(packettotransmit);
    }
    linktemp[inputPortNumber][destinationPort[packettotransmit]]=1;
    }
} 

//  Returns the mean packet delay
double meanpacketdelay(){
    double ans=0;
    int transmittedPackets = 0;
    for(int i=0;i<packetId;i++){
        if(completionTime[i]){
            transmittedPackets++;
         ans+=completionTime[i]-arrivalTime[i];
        }
    }
    ans/=transmittedPackets;
    for(int i=0;i<packetId;i++){
        if(completionTime[i]){
         standarddevpacketdelay+=(completionTime[i]-arrivalTime[i]-ans)*(completionTime[i]-arrivalTime[i]-ans);
        }
    }
    standarddevpacketdelay/=transmittedPackets;
    standarddevpacketdelay=sqrt(standarddevpacketdelay);
    return ans;
}

//  Adds up the temporary links value after every iteration
void updatelink(){
    for(int i=0;i<numOfInputPorts;i++){
        for(int j=0;j<numOfOutputPorts;j++){
            link[i][j]+=linktemp[i][j];
        }
    }
}
// Returns the average link utilization value
double finalizelink(){
    double ans=0;
    for(int i=0;i<numOfInputPorts;i++){
        for(int j=0;j<numOfOutputPorts;j++){
            link[i][j]/=(double)maxTimeSlots;
            ans+=link[i][j];
        }
    }
    ans/=((double)numOfInputPorts);
    return ans;  
}
// Performs the INQ Scheduling
void INQScheduling(){
     for(int i = 0; i < numOfInputPorts; i++){
         if(inputPort[i].size() > 0){
             contention[destinationPort[inputPort[i].front()]].push_back(inputPort[i].front());
         }
     }
     for(int i = 0; i < numOfOutputPorts; i++){
         if(contention[i].size() == 1){
             PlaceInOutputBuffer(arrivalPort[contention[i][0]]);
         }else if(contention[i].size()>1){
             int index = uniform_int_distribution <int> (0,(int)contention[i].size()-1)(rng);
             PlaceInOutputBuffer(arrivalPort[contention[i][index]]);
         }
     }
     for(int i = 0; i < numOfInputPorts; i++){
             contention[i].clear();
     }
}
// comparator function for sorting the packets contending for a given output port by arrival time
bool cmp(int id1, int id2){
    return arrivalTime[id1] <  arrivalTime[id2];
}

// Performs the KOUQ Scheduling
void KOUQScheduling(){
    for(int i = 0; i < numOfInputPorts; i++){
         if(inputPort[i].size() > 0){
             contention[destinationPort[inputPort[i].front()]].push_back(inputPort[i].front());
         }
     }
     int cntr=0;
     for(int i = 0; i < numOfOutputPorts; i++){
         sort(contention[i].begin(), contention[i].end(), cmp);
         if(contention[i].size() <= K){
             for (int j = 0; j < contention[i].size(); j++){
                 PlaceInOutputBuffer(arrivalPort[contention[i][j]]);
             }
         }
         else{
             cntr++;
             vector<int> randSelectedPackets;
             for(int j = 0; j < K; j++){
                 int randIndex = uniform_int_distribution <int> (0, contention[i].size()-1)(rng);
                 randSelectedPackets.push_back(contention[i][randIndex]);
                 swap(contention[i][randIndex], contention[i][(int)contention[i].size()-1]);
                 contention[i].pop_back();
             }
             sort(randSelectedPackets.begin(), randSelectedPackets.end(), cmp);
             for(int j = 0; j < K; j++){
                 PlaceInOutputBuffer(arrivalPort[randSelectedPackets[j]]);
             }
             for(int j = 0; j < contention[i].size(); j++){
                 inputPort[arrivalPort[contention[i][j]]].pop();
             }
         }
         kouqdropprob+=(double)cntr/(double)numOfOutputPorts;
     }
    for(int i = 0; i < numOfInputPorts; i++){
             contention[i].clear();
    }
}

// Sets KOUQ drop probability
void setkouqdropprob(){
    kouqdropprob=kouqdropprob/maxTimeSlots;
}
bool request[1000][1000],grant[1000][1000],accept[1000][1000]; //stores which input i has sent/recieved 
// request/grant/accept from which output j

// Performs iSLIP Scheduling
void iSLIPScheduling(){
     for(int i=0;i<numOfInputPorts;i++){
         for(int j=0;j<inputPort[i].size();j++){
             int packet=inputPort[i].front();
             inputPort[i].pop();
             VOQinputPort[i][destinationPort[packet]].push(packet);
             inputPort[i].push(packet);
         }
     }
     unordered_map<int,int>busyinput,busyoutput;
     //Input/Output port pointers
     int RRInPtr[numOfInputPorts]={};
     int RROutPtr[numOfOutputPorts]={};
     int numOfIterations=numOfInputPorts;
     while(numOfIterations--){

       for(int i=0;i<numOfInputPorts;i++){
           for(int j=0;j<numOfOutputPorts;j++){
               request[i][j]=0;
               grant[i][j]=0;
               accept[i][j]=0;
           }
       }
       //request phase 
       for(int i=0;i<numOfInputPorts;i++){
           for(int j=0;j<numOfOutputPorts;j++){
               if(VOQinputPort[i][j].size() && busyinput[i]==0 && busyoutput[j]==0){
                   request[i][j]=1;
               }
           }
       }
       
       //grant phase 
       for(int j=0;j<numOfOutputPorts;j++){
         for(int i=0;i<numOfInputPorts;i++){
             if(request[(i+RROutPtr[j])%numOfInputPorts][j]){
                grant[(i+RROutPtr[j])%numOfInputPorts][j]=1;
                break;
             }else{

             }
         }
       }

       //accept phase 
       for(int i=0;i<numOfInputPorts;i++){
           for(int j=0;j<numOfOutputPorts;j++){
               if(grant[i][(j+RRInPtr[i])%numOfOutputPorts]){
                 accept[i][(j+RRInPtr[i])%numOfOutputPorts]=1;
                 int packet=VOQinputPort[i][(j+RRInPtr[i])%numOfOutputPorts].front();
                 VOQinputPort[i][(j+RRInPtr[i])%numOfOutputPorts].pop();
                 linktemp[i][(j+RRInPtr[i])%numOfOutputPorts]=1;
                 busyinput[i]=1;
                 busyoutput[(j+RRInPtr[i])%numOfOutputPorts]=1;
                 //deleting from queue
                 int sizeofq=inputPort[i].size();
                 while(sizeofq--){
                     int temp=inputPort[i].front();
                     inputPort[i].pop();
                     if(temp!=packet){
                     inputPort[i].push(temp);
                     }else{

                     }
                 }

                 outputPort[(j+RRInPtr[i])%numOfOutputPorts].push(packet);
                 if(numOfIterations==numOfInputPorts){
                 RRInPtr[i]=(j+RRInPtr[i]+1)%numOfOutputPorts;
                 RROutPtr[(j+RRInPtr[i])%numOfOutputPorts]=(i+1)%numOfInputPorts;
                 }
                 break;
               }else{

               }
           }
       }
     }
     for(int i=0;i<numOfInputPorts;i++){
         for(int j=0;j<numOfOutputPorts;j++){
            while(VOQinputPort[i][j].size()){
                VOQinputPort[i][j].pop();
            }
         }
     }
}

void resetValues(){
    timer=0;
    packetId=0;
    kouqdropprob=0;
    standarddevpacketdelay=0;
    //special
    for(int i=0;i<50;i++){
        while(inputPort[i].size()){inputPort[i].pop();}
        while(outputPort[i].size()){outputPort[i].pop();}
    }
    for(int i=0;i<50;i++){contention[i].clear();}
    for(int i=0;i<50;i++){
        for(int j=0;j<50;j++){
            while(VOQinputPort[i][j].size()){VOQinputPort[i][j].pop();}
        }
    }
    for(int i=0;i<50;i++){
        for(int j=0;j<50;j++){link[i][j]=0;linktemp[i][j]=0;}
    }
    arrivalPort.clear();
    destinationPort.clear();
    arrivalTime.clear();
    completionTime.clear();
    startTime.clear();
}

int main(int argc, char* argv[]){
    string outputFile = "output.txt";
    resetValues();
    string s;
    for(int i = 0; i < argc; i++){
        s = argv[i];
        if(s == "-N"){
            numOfInputPorts = stoi(argv[i+1]);
            numOfOutputPorts = stoi(argv[i+1]);
            break;
        }
    }

    for(int i = 0; i < argc; i++){
        s = argv[i];
        if(s == "-B"){
            bufferSize = stoi(argv[i+1]);
            break;
        }
    }

    for(int i = 0; i < argc; i++){
        s = argv[i];
        if(s == "-p"){
            packetGenProb = stod(argv[i+1]);
            break;
        }
    }
    
    for(int i = 0; i < argc; i++){
        s = argv[i];
        if(s == "-queue"){
            policy = argv[i+1];
            break;
        }
    }
    
    for(int i = 0; i < argc; i++){
        s = argv[i];
        if(s == "-K"){
            K = stoi(argv[i+1]);
            break;
        }
    }

    for(int i = 0; i < argc; i++){
        s = argv[i];
        if(s == "-out"){
            outputFile = argv[i+1];
            break;
        }
    }

    for(int i = 0; i < argc; i++){
        s = argv[i];
        if(s == "-T"){
            maxTimeSlots = stoi(argv[i+1]);
            break;
        }
    }

    //fstream fout;
    //fout.open(outputFile,ios::out);
    ofstream fout;
    fout.open(outputFile, std::ios_base::app);
    int scaledProb=1000*packetGenProb;
    while(timer++<maxTimeSlots){
        if(policy=="INQ"){
            INQScheduling();
            updatelink();
        }else if(policy=="KOUQ"){
            KOUQScheduling();
            updatelink();
        }else if(policy=="iSLIP"){
            iSLIPScheduling();
            updatelink();
        }
        GenerateTraffic(scaledProb);
        for(int i=0;i<numOfOutputPorts;i++){
            if(outputPort[i].size()>0){
                completionTime[outputPort[i].front()]=timer;
                outputPort[i].pop();
            }
            
        }
    }
    fout<<numOfInputPorts<<"   ";
    fout<<packetGenProb<<"   ";
    fout<<policy<<"   ";
    fout<<meanpacketdelay()<<"   ";
    fout<<standarddevpacketdelay<<"   ";
    fout<<finalizelink()<<"\n";

    fout.close();

}