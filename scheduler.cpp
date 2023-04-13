#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
using namespace std; 

// sample input: p0 2 6 p1 5 2 p2 1 8 p3 0 3 p4 4 4 TYPE

double processCount = 0;
double responseCount = 0;
double turnaroundCount = 0;
vector<string> accessedProcess;

struct process{
    string processName;
	int arrivalTime;
	int executionTime;
};

class scheduler{
	public:
    vector <process>inputProcesses;
	void FCFS();
	void preemptiveSJF();
};

void scheduler::FCFS(){
    // using multimap since processes could have identical arrival times, aka duplicate keys
    multimap<int, process> sortedProcesses; 
    for (auto &p: inputProcesses){
        // starting turnaround calculations, formula: turnaround = final exit time - arrival time
        turnaroundCount -= p.arrivalTime; 
        sortedProcesses.insert(pair<int, process>(p.arrivalTime, p)); // sorted by arrival time
    }

    int currentTime = 0;
    int stopTime = 0;

    for (auto & p : sortedProcesses){
        process & pr = p.second;

        // next process can't start at currentTime if it has yet to arrive
        if (currentTime < pr.arrivalTime){
            currentTime = pr.arrivalTime;
        }

        cout << "-------------------" << endl << "Schedule for process " << pr.processName << endl;
        stopTime = currentTime + pr.executionTime;
        cout << "Start time " << currentTime << " Stop time " << stopTime << endl;

        // response time = processGetsCPU - arrivalTime
        responseCount += currentTime - pr.arrivalTime;

        // process exits, we add exit time to turnaround
        turnaroundCount += stopTime; 

        // update currentTime
        currentTime = stopTime;
    }
    cout << "-------------------" << endl;
    cout << "Throughput: " << processCount/stopTime << " jobs/second" << endl;
    cout << "Average response time: " << responseCount/processCount << " seconds" << endl;
    cout << "Average turnaround time: " << turnaroundCount/processCount << " seconds" << endl;
}

void scheduler::preemptiveSJF(){
    multimap<int, process> sortedProcesses; 
    for (auto &p: inputProcesses){
        // starting turnaround calculations
        turnaroundCount -= p.arrivalTime;
        sortedProcesses.insert(pair<int, process>(p.arrivalTime, p)); // sorted by arrival time
    }

    int currentTime = 0;
    int stopTime = 0;

    // new map for time left for each process, sorted by remaining execution time
    multimap<int, process> workingTime; 

    // while we have processes in either multimaps
    while (!sortedProcesses.empty() || !workingTime.empty()){ 
        if (!sortedProcesses.empty()){
            process nextJob = sortedProcesses.begin()->second;
            if (workingTime.empty() || currentTime >= nextJob.arrivalTime) { // first process || new process in "queue"/has arrived
                currentTime = nextJob.arrivalTime;
                sortedProcesses.erase(sortedProcesses.begin());
                workingTime.insert(pair<int, process>(nextJob.executionTime, nextJob));
                continue;
            }
            // we stop current process at next job's arrival to compare their execution times
            stopTime = nextJob.arrivalTime;    
        }
        else{
            // workingTime is sorted, we add the shortest execution time to currentTime
            stopTime = currentTime + workingTime.begin()->second.executionTime;
        }

        process currentJob = workingTime.begin()->second;
        workingTime.erase(workingTime.begin());

        // if there is a "gap" until nextJob's arrival, we update stopTime to no longer equal nextJob arrival
        // or if process executes completely (no need to reinsert into map)
        if (currentTime + currentJob.executionTime <= stopTime){ 
            stopTime = currentTime + currentJob.executionTime;
        }
        else { 
            // another process present before currentJob.executionTime can be added to currentTime
            // we remove from currentTime how much has been executed, and insert again with updated value
            currentJob.executionTime -= (stopTime-currentTime);
            workingTime.insert(pair<int, process>(currentJob.executionTime, currentJob));
        }
        cout << "-------------------" << endl;
        cout << "Executing process " << currentJob.processName << endl;        
        cout << "Start time " << currentTime << "  Stop time " << stopTime << endl;

        // for turnaround time, we only add stop time once process has exited completely
        // if it isn't in working time, it has exited
        bool found = false;
        multimap<int, process>::iterator it;
        for(it=workingTime.begin();it!=workingTime.end();++it) {
            if (it->second.processName == currentJob.processName){
                found = true;
                break;
            }
        }
        if (!found){ 
            turnaroundCount += stopTime;
        }

        // for response time, we only add current time for instance of first CPU access
        found = false;
        for (auto &str : accessedProcess){
           if (str == currentJob.processName){
            found = true;
            break;
           }
        }
        if (!found){ 
            responseCount += currentTime - currentJob.arrivalTime;
            accessedProcess.push_back(currentJob.processName);
        }

        // updating current time
        currentTime = stopTime;

    }
    cout << "-------------------" << endl;
    cout << "Throughput: " << processCount/stopTime << " jobs/second" << endl;
    cout << "Average response time: " << responseCount/processCount << " seconds" << endl;
    cout << "Average turnaround time: " << turnaroundCount/processCount << " seconds" << endl;
}

int main(int argc, char **argv){

    // creating class object to store all processes and apply appropriate function
    scheduler s;

    // in each iteration we identify the elements of a process from the input
    for (int i = 1; i < argc - 1; i++){
        process p;
        p.processName = argv[i++];
        p.arrivalTime = atoi(argv[i++]); //  ascii to int conversion
        p.executionTime = atoi(argv[i]);
        s.inputProcesses.push_back(p);
        processCount++;
    }

    // check last argument in input, should tell us the scheduling algorithm
    // if it doesn't --> error message and exit
    string type = argv[argc - 1];
    for (auto & c: type) c = toupper(c);

    if (type == string("FCFS")){
        s.FCFS();
    }
    else if (type == string("SJF")){
        s.preemptiveSJF();
    }
    else{
        cout << "Error: Unable to identify type." << endl;
    }

    return 0;
}
