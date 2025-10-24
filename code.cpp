
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
#include <climits>
#include <queue> // For Round Robin ready queue

using namespace std;

// Class to represent a Process
class Process {
public:
    int pid;        // Process ID
    int at;         // Arrival Time
    int bt;         // Original Burst Time
    int rem_bt;     // Remaining Burst Time (Crucial for SRTF and RR)
    int ct;         // Completion Time
    int tat;        // Turn Around Time
    int wt;         // Waiting Time
    int priority;   // Priority (Smaller number = Higher Priority)
    
    // Constructor
    Process(int id, int a, int b, int p) {
        pid = id;
        at = a;
        bt = b;
        rem_bt = b; // Initialize remaining time to original burst time
        priority = p;
        ct = tat = wt = 0;
    }
};

// Helper function to print results in a structured table
void printResults(vector<Process> &procs, const vector<int>& timeline, const vector<string>& blocks, const string& algorithmName) {
    int fixedWidth = 8;
    int idleTime = 0;
    float totalTAT = 0, totalWT = 0;
    int n = procs.size();

    // Re-sort processes by PID for a clean result table display
    sort(procs.begin(), procs.end(), [](Process &a, Process &b){
        return a.pid < b.pid;
    });

    cout << "\n---------------------------------------------------------------\n";
    cout << "\t\t" << algorithmName << " Results\n";
    cout << "---------------------------------------------------------------\n";

    cout << "\nGantt Chart (" << algorithmName << "):\n";
    
    // ---- print blocks ----
    for (const auto &b : blocks) {
        cout << "| " << left << setw(fixedWidth - 2) << b;
    }
    cout << "|\n";
    
    // ---- print timeline ----
    for (size_t i = 0; i < timeline.size(); i++) {
        cout << left << setw(fixedWidth) << timeline[i];
    }
    cout << "\n\n";

    // Print table header based on algorithm
    if (algorithmName.find("Priority") != string::npos) {
        cout << "PID\tAT\tBT\tPRI\tCT\tTAT\tWT\n";
        cout << "-------------------------------------------------------------\n";
    } else {
        cout << "PID\tAT\tBT\tCT\tTAT\tWT\n";
        cout << "------------------------------------------------\n";
    }

    // Print process metrics and calculate totals
    for (auto &p : procs) {
        totalTAT += p.tat;
        totalWT += p.wt;
        cout << p.pid << "\t" << p.at << "\t" << p.bt;
        if (algorithmName.find("Priority") != string::npos) {
            cout << "\t" << p.priority;
        }
        cout << "\t" << p.ct << "\t" << p.tat << "\t" << p.wt << "\n";
    }

    // Idle time calculation
    for (size_t i = 0; i < blocks.size(); i++) {
        if (blocks[i] == "IDLE" && i + 1 < timeline.size()) {
            idleTime += timeline[i+1] - timeline[i];
        }
    }
    
    cout << fixed << setprecision(2);
    cout << "\nAverage Turn Around Time: " << totalTAT / n << " units\n";
    cout << "Average Waiting Time: " << totalWT / n << " units\n";
    cout << "Total CPU Idle Time: " << idleTime << " units\n";
}

// -----------------------------------------------------------------------------
// FCFS Scheduling
// -----------------------------------------------------------------------------

void FCFS(vector<Process> procs) {
    // FCFS rule: Sort by Arrival Time (AT)
    sort(procs.begin(), procs.end(), [](Process &a, Process &b){
        return a.at < b.at;
    });

    int time = 0;
    vector<int> timeline;
    vector<string> blocks;

    timeline.push_back(0);

    for (auto &p : procs) {
        if (p.at > time) {
            // CPU is IDLE until the process arrives
            blocks.push_back("IDLE");
            time = p.at; // Advance time to process arrival
            timeline.push_back(time);
        }

        // Execute the process
        blocks.push_back("P" + to_string(p.pid));
        time += p.bt;
        timeline.push_back(time);

        // Calculate metrics
        p.ct = time;
        p.tat = p.ct - p.at;
        p.wt = p.tat - p.bt;
    }
    
    printResults(procs, timeline, blocks, "FCFS");
}

// -----------------------------------------------------------------------------
// SJF Non-Preemptive Scheduling
// -----------------------------------------------------------------------------

void SJF(vector<Process> procs) {
    int n = procs.size();
    vector<bool> completed(n, false);
    int time = 0, completedCount = 0;
    vector<int> timeline;
    vector<string> blocks;
    timeline.push_back(0);

    while (completedCount < n) {
        int idx = -1;
        int minBT = INT_MAX;

        // 1. Find the process with minimum BT that has arrived (at <= time) and is not completed
        for (int i = 0; i < n; i++) {
            if (!completed[i] && procs[i].at <= time) {
                if (procs[i].bt < minBT) {
                    minBT = procs[i].bt;
                    idx = i;
                }
                // Tie-breaker: FCFS for equal burst times
                else if (procs[i].bt == minBT) {
                    if (idx != -1 && procs[i].at < procs[idx].at) {
                        idx = i;
                    }
                }
            }
        }

        if (idx == -1) {
            // 2. CPU is IDLE
            int next_arrival_time = INT_MAX;
            for(int i = 0; i < n; i++) {
                if(!completed[i]) {
                    next_arrival_time = min(next_arrival_time, procs[i].at);
                }
            }

            if (next_arrival_time != INT_MAX) {
                blocks.push_back("IDLE");
                time = next_arrival_time; // Jump time directly
                timeline.push_back(time);
            } else {
                break;
            }
        } else {
            // 3. Execute the shortest job (non-preemptive)
            
            // Execute the process
            blocks.push_back("P" + to_string(procs[idx].pid));
            time += procs[idx].bt;
            timeline.push_back(time);

            // Calculate metrics and mark as completed
            procs[idx].ct = time;
            procs[idx].tat = procs[idx].ct - procs[idx].at;
            procs[idx].wt = procs[idx].tat - procs[idx].bt;
            completed[idx] = true;
            completedCount++;
        }
    }

    printResults(procs, timeline, blocks, "SJF - Non Preemptive");
}

// -----------------------------------------------------------------------------
// Priority Scheduling Non-Preemptive
// -----------------------------------------------------------------------------

void PriorityScheduling(vector<Process> procs) {
    int n = procs.size();
    vector<bool> completed(n, false);
    int time = 0, completedCount = 0;
    vector<int> timeline;
    vector<string> blocks;
    timeline.push_back(0);

    while (completedCount < n) {
        int idx = -1;
        // Selection criteria: MINIMUM priority value (highest priority)
        int minPriority = INT_MAX;

        // 1. Find the process with minimum Priority that has arrived and is not completed
        for (int i = 0; i < n; i++) {
            if (!completed[i] && procs[i].at <= time) {
                if (procs[i].priority < minPriority) {
                    minPriority = procs[i].priority;
                    idx = i;
                }
                // Tie-breaker: FCFS for equal priorities
                else if (procs[i].priority == minPriority) {
                    if (idx != -1 && procs[i].at < procs[idx].at) {
                        idx = i;
                    }
                }
            }
        }

        if (idx == -1) {
            // 2. CPU is IDLE
            int next_arrival_time = INT_MAX;
            for(int i = 0; i < n; i++) {
                if(!completed[i]) {
                    next_arrival_time = min(next_arrival_time, procs[i].at);
                }
            }

            if (next_arrival_time != INT_MAX) {
                blocks.push_back("IDLE");
                time = next_arrival_time;
                timeline.push_back(time);
            } else {
                break;
            }
        } else {
            // 3. Execute the highest priority job (non-preemptive)
            
            blocks.push_back("P" + to_string(procs[idx].pid));
            time += procs[idx].bt;
            timeline.push_back(time);

            // Calculate metrics and mark as completed
            procs[idx].ct = time;
            procs[idx].tat = procs[idx].ct - procs[idx].at;
            procs[idx].wt = procs[idx].tat - procs[idx].bt;
            completed[idx] = true;
            completedCount++;
        }
    }

    printResults(procs, timeline, blocks, "Priority Scheduling (Non-Preemptive)");
}

// -----------------------------------------------------------------------------
// SRTF Preemptive Scheduling
// -----------------------------------------------------------------------------

void SRTF(vector<Process> procs) {
    int n = procs.size();
    int time = 0;
    int completedCount = 0;
    
    // Ensure all rem_bt are correctly initialized
    for(auto& p : procs) {
        p.rem_bt = p.bt;
    }

    // Gantt Chart tracking variables
    vector<int> timeline = {0};
    vector<string> blocks = {};
    string last_block_id = ""; // Tracks the process that ran in the previous time unit

    while (completedCount < n) {
        int min_rem_bt = INT_MAX;
        int shortest_idx = -1;
        
        // 1. Find the process with the shortest remaining time in the ready queue
        for (int i = 0; i < n; i++) {
            if (procs[i].at <= time && procs[i].rem_bt > 0) {
                if (procs[i].rem_bt < min_rem_bt) {
                    min_rem_bt = procs[i].rem_bt;
                    shortest_idx = i;
                }
                // Tie-breaker: FCFS for equal remaining times
                else if (procs[i].rem_bt == min_rem_bt) {
                    if (shortest_idx != -1 && procs[i].at < procs[shortest_idx].at) {
                        shortest_idx = i;
                    }
                }
            }
        }

        if (shortest_idx == -1) {
            // 2. CPU is IDLE
            int next_arrival_time = INT_MAX;
            bool found_next = false;
            
            for(int i = 0; i < n; i++) {
                if(procs[i].rem_bt > 0) {
                    next_arrival_time = min(next_arrival_time, procs[i].at);
                    found_next = true;
                }
            }

            if (found_next) {
                // Only push "IDLE" if the previous block wasn't IDLE
                if (last_block_id != "IDLE") {
                    blocks.push_back("IDLE");
                    timeline.push_back(time);
                }
                time = next_arrival_time; // Jump time
                last_block_id = "IDLE";
            } else {
                break;
            }
        }
        else {
            // 3. Process Execution for 1 unit
            string current_block_id = "P" + to_string(procs[shortest_idx].pid);
            
            // Preemption Check: If the process is changing (new shortest or transition from IDLE)
            if (current_block_id != last_block_id) {
                blocks.push_back(current_block_id);
                // Push current time only if it's the start of a new block
                if (last_block_id != "IDLE") {
                    timeline.push_back(time);
                }
            }
            
            last_block_id = current_block_id;
            
            // Execute for 1 unit
            procs[shortest_idx].rem_bt--;
            time++;

            // 4. Completion Check
            if (procs[shortest_idx].rem_bt == 0) {
                completedCount++;
                
                // Finalize metrics
                procs[shortest_idx].ct = time;
                procs[shortest_idx].tat = procs[shortest_idx].ct - procs[shortest_idx].at;
                procs[shortest_idx].wt = procs[shortest_idx].tat - procs[shortest_idx].bt;
                
                // Mark completion on the Gantt chart
                timeline.push_back(time);
                last_block_id = ""; // Reset block ID to trigger new block on next iteration
            }
        }
    }

    // --- Gantt Chart Cleanup (Merging consecutive identical blocks for cleaner visual) ---
    vector<int> final_timeline;
    vector<string> final_blocks;
    if (!blocks.empty()) {
        final_blocks.push_back(blocks[0]);
        final_timeline.push_back(timeline[0]);
        
        for (size_t i = 1; i < blocks.size(); ++i) {
            if (blocks[i] != final_blocks.back()) {
                final_timeline.push_back(timeline[i]);
                final_blocks.push_back(blocks[i]);
            }
        }
        if (final_timeline.back() != timeline.back()) {
            final_timeline.push_back(timeline.back());
        }
    }

    printResults(procs, final_timeline, final_blocks, "SRTF - Preemptive SJF");
}

// -----------------------------------------------------------------------------
// Round Robin Scheduling
// -----------------------------------------------------------------------------

void RoundRobin(vector<Process> procs, int quantum) {
    int n = procs.size();
    int time = 0;
    int completedCount = 0;
    
    // Set up the ready queue (stores process indices)
    queue<int> readyQueue;
    vector<bool> inQueue(n, false); // To track if a process is already in the queue
    
    // Sort processes by arrival time to easily check for arrivals
    sort(procs.begin(), procs.end(), [](Process &a, Process &b){
        return a.at < b.at;
    });

    // Remap PIDs to indices for queue/vector access
    vector<int> pid_to_idx(procs.back().pid + 1, -1);
    for(int i = 0; i < n; ++i) {
        pid_to_idx[procs[i].pid] = i;
        procs[i].rem_bt = procs[i].bt; // Reset remaining burst time
    }
    
    // Gantt Chart tracking variables
    vector<int> timeline = {0};
    vector<string> blocks = {};
    string last_block_id = "";
    int next_proc_to_arrive = 0; // Index of the next process to check for arrival

    while (completedCount < n) {
        
        // 1. Add all newly arrived processes to the ready queue (FCFS order)
        for (int i = next_proc_to_arrive; i < n; ++i) {
            if (procs[i].at <= time && !inQueue[i]) {
                readyQueue.push(i);
                inQueue[i] = true;
                next_proc_to_arrive = i + 1; // Optimization: next arrival check starts here
            } else if (procs[i].at > time) {
                break; // Processes are sorted by AT, so no need to check further
            }
        }

        if (readyQueue.empty()) {
            // 2. CPU is IDLE
            int next_arrival_time = INT_MAX;
            bool found_next = false;
            
            // Find the minimum arrival time of any uncompleted, non-queued process
            for(int i = next_proc_to_arrive; i < n; i++) {
                 if (procs[i].rem_bt > 0) {
                    next_arrival_time = procs[i].at;
                    found_next = true;
                    break;
                 }
            }

            if (found_next) {
                // Only push "IDLE" if the previous block wasn't IDLE
                if (last_block_id != "IDLE") {
                    blocks.push_back("IDLE");
                    timeline.push_back(time);
                }
                time = next_arrival_time; // Jump time
                last_block_id = "IDLE";
            } else {
                break; // All processes completed or no more processes to arrive
            }
        } else {
            // 3. Execute the process at the front of the queue
            int current_proc_idx = readyQueue.front();
            readyQueue.pop();
            inQueue[current_proc_idx] = false; // Mark as not in queue (it's running)

            int execution_time = min(quantum, procs[current_proc_idx].rem_bt);
            string current_block_id = "P" + to_string(procs[current_proc_idx].pid);
            
            // Start of a new block in Gantt Chart
            if (current_block_id != last_block_id) {
                blocks.push_back(current_block_id);
                // The time will be pushed at the start of the block or after IDLE
                if (last_block_id != "IDLE") {
                    timeline.push_back(time);
                }
            }
            last_block_id = current_block_id;

            // Execute
            procs[current_proc_idx].rem_bt -= execution_time;
            time += execution_time;

            // 4. Check for arrivals *during* execution (important for RR)
            for (int i = next_proc_to_arrive; i < n; ++i) {
                if (procs[i].at <= time && !inQueue[i]) {
                    readyQueue.push(i);
                    inQueue[i] = true;
                    next_proc_to_arrive = i + 1;
                } else if (procs[i].at > time) {
                    break;
                }
            }

            if (procs[current_proc_idx].rem_bt == 0) {
                // Process Completed
                completedCount++;
                procs[current_proc_idx].ct = time;
                procs[current_proc_idx].tat = procs[current_proc_idx].ct - procs[current_proc_idx].at;
                procs[current_proc_idx].wt = procs[current_proc_idx].tat - procs[current_proc_idx].bt;
                
                timeline.push_back(time);
                last_block_id = ""; // Force a new block start after completion
            } else {
                // Process Preempted (not completed)
                readyQueue.push(current_proc_idx);
                inQueue[current_proc_idx] = true; // Put back into the queue
                
                timeline.push_back(time);
                last_block_id = ""; // Force a new block start after preemption
            }
        }
    }
    
    // --- Final process vector must be re-sorted by PID for printResults ---
    sort(procs.begin(), procs.end(), [](Process &a, Process &b){
        return a.pid < b.pid;
    });

    // --- Gantt Chart Cleanup (Merging consecutive identical blocks for cleaner visual) ---
    vector<int> final_timeline;
    vector<string> final_blocks;
    if (!blocks.empty()) {
        final_blocks.push_back(blocks[0]);
        final_timeline.push_back(timeline[0]);
        
        for (size_t i = 1; i < blocks.size(); ++i) {
            if (blocks[i] != final_blocks.back()) {
                final_timeline.push_back(timeline[i]);
                final_blocks.push_back(blocks[i]);
            }
        }
        if (final_timeline.back() != timeline.back()) {
            final_timeline.push_back(timeline.back());
        }
    }

    printResults(procs, final_timeline, final_blocks, "Round Robin (RR)");
}

// -----------------------------------------------------------------------------
// Main Driver Function
// -----------------------------------------------------------------------------

int main() {
    // Set output formatting
    cout << fixed << setprecision(2);

    int n;
    cout << "Enter number of processes: ";
    if (!(cin >> n) || n <= 0) {
        cout << "Invalid number of processes.\n";
        return 1;
    }

    vector<Process> procs_input;
    for (int i = 0; i < n; i++) {
        int at, bt, pri = 0;
        cout << "\nEnter details for P" << i+1 << ":\n";
        cout << "Arrival Time (AT): ";
        if (!(cin >> at) || at < 0) return 1;
        cout << "Burst Time (BT): ";
        if (!(cin >> bt) || bt <= 0) return 1;
        cout << "Priority (PRI): ";
        if (!(cin >> pri) || pri < 0) return 1;

        procs_input.push_back(Process(i+1, at, bt, pri));
    }

    cout << "\n===============================================\n";
    cout << "Select Algorithm:\n";
    cout << "1. First Come, First Served (FCFS)\n";
    cout << "2. Shortest Job First (SJF - Non Preemptive)\n";
    cout << "3. Priority Scheduling (Non-Preemptive)\n";
    cout << "4. Shortest Remaining Time First (SRTF - Preemptive SJF)\n";
    cout << "5. Round Robin (RR)\n"; // New option
    cout << "Choice: ";

    int choice;
    if (!(cin >> choice)) {
        cout << "Invalid choice.\n";
        return 1;
    }

    if (choice == 1) FCFS(procs_input);
    else if (choice == 2) SJF(procs_input);
    else if (choice == 3) PriorityScheduling(procs_input);
    else if (choice == 4) SRTF(procs_input);
    else if (choice == 5) {
        int quantum;
        cout << "Enter Time Quantum for Round Robin: ";
        if (!(cin >> quantum) || quantum <= 0) {
            cout << "Invalid Time Quantum.\n";
            return 1;
        }
        RoundRobin(procs_input, quantum);
    }
    else cout << "Invalid choice.\n";

    return 0;
}
