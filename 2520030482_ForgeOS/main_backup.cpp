#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <string>
#include <climits>

using namespace std;

struct Process {
    string pid;
    int arrivalTime;
    int burstTime;
    int priority;

    int completionTime = 0;
    int turnaroundTime = 0;
    int waitingTime = 0;
    int responseTime = -1;
    int remainingTime = 0;
};

struct Segment {
    string pid;
    int startTime;
    int endTime;
};

struct Summary {
    double avgWT;
    double avgTAT;
    double avgRT;
};

void addSegment(vector<Segment>& gantt, string pid, int start, int end) {
    if (start == end)
        return;

    if (!gantt.empty() &&
        gantt.back().pid == pid &&
        gantt.back().endTime == start) {
        gantt.back().endTime = end;
    } else {
        gantt.push_back({pid, start, end});
    }
}

void printResults(const vector<Process>& p, string algorithm) {

    cout << "\n============================================================\n";
    cout << "                 " << algorithm << " RESULTS\n";
    cout << "============================================================\n";

    cout << left
         << setw(8) << "PID"
         << setw(8) << "AT"
         << setw(8) << "BT";

    if (algorithm == "Priority Scheduling")
        cout << setw(8) << "PR";

    cout << setw(8) << "CT"
         << setw(8) << "TAT"
         << setw(8) << "WT"
         << setw(8) << "RT"
         << "\n";

    cout << "------------------------------------------------------------\n";

    double totalWT = 0;
    double totalTAT = 0;
    double totalRT = 0;

    vector<Process> sorted = p;

    sort(sorted.begin(), sorted.end(),
         [](const Process& a, const Process& b) {
             return a.pid < b.pid;
         });

    for (const auto& x : sorted) {

        cout << left
             << setw(8) << x.pid
             << setw(8) << x.arrivalTime
             << setw(8) << x.burstTime;

        if (algorithm == "Priority Scheduling")
            cout << setw(8) << x.priority;

        cout << setw(8) << x.completionTime
             << setw(8) << x.turnaroundTime
             << setw(8) << x.waitingTime
             << setw(8) << x.responseTime
             << "\n";

        totalWT += x.waitingTime;
        totalTAT += x.turnaroundTime;
        totalRT += x.responseTime;
    }

    cout << "------------------------------------------------------------\n";

    double avgWT = totalWT / p.size();
    double avgTAT = totalTAT / p.size();
    double avgRT = totalRT / p.size();

    cout << fixed << setprecision(2);

    cout << "Average Waiting Time    : " << avgWT << "\n";
    cout << "Average Turnaround Time : " << avgTAT << "\n";
    cout << "Average Response Time   : " << avgRT << "\n";
}

Summary calculateSummary(const vector<Process>& p) {

    double totalWT = 0;
    double totalTAT = 0;
    double totalRT = 0;

    for (const auto& x : p) {
        totalWT += x.waitingTime;
        totalTAT += x.turnaroundTime;
        totalRT += x.responseTime;
    }

    return {
        totalWT / p.size(),
        totalTAT / p.size(),
        totalRT / p.size()
    };
}

vector<Process> fcfs(vector<Process> p, vector<Segment>& gantt) {

    sort(p.begin(), p.end(),
         [](const Process& a, const Process& b) {

             if (a.arrivalTime == b.arrivalTime)
                 return a.pid < b.pid;

             return a.arrivalTime < b.arrivalTime;
         });

    int currentTime = 0;

    for (auto& x : p) {

        if (currentTime < x.arrivalTime) {

            addSegment(
                gantt,
                "IDLE",
                currentTime,
                x.arrivalTime
            );

            currentTime = x.arrivalTime;
        }

        int startTime = currentTime;

        currentTime += x.burstTime;

        addSegment(
            gantt,
            x.pid,
            startTime,
            currentTime
        );

        x.completionTime = currentTime;

        x.turnaroundTime =
            x.completionTime - x.arrivalTime;

        x.waitingTime =
            x.turnaroundTime - x.burstTime;

        x.responseTime =
            startTime - x.arrivalTime;
    }

    printResults(p, "FCFS");

    return p;
}

vector<Process> sjf(vector<Process> p, vector<Segment>& gantt) {

    int n = p.size();

    vector<bool> done(n, false);

    int completed = 0;
    int currentTime = 0;

    while (completed < n) {

        int index = -1;
        int shortestBurst = INT_MAX;

        for (int i = 0; i < n; i++) {

            if (!done[i] &&
                p[i].arrivalTime <= currentTime) {

                if (p[i].burstTime < shortestBurst) {

                    shortestBurst = p[i].burstTime;
                    index = i;

                } else if (
                    p[i].burstTime == shortestBurst &&
                    p[i].arrivalTime < p[index].arrivalTime) {

                    index = i;
                }
            }
        }

        if (index == -1) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {

                if (!done[i])
                    nextArrival =
                        min(nextArrival, p[i].arrivalTime);
            }

            addSegment(
                gantt,
                "IDLE",
                currentTime,
                nextArrival
            );

            currentTime = nextArrival;

            continue;
        }

        int startTime = currentTime;

        currentTime += p[index].burstTime;

        addSegment(
            gantt,
            p[index].pid,
            startTime,
            currentTime
        );

        p[index].completionTime = currentTime;

        p[index].turnaroundTime =
            currentTime - p[index].arrivalTime;

        p[index].waitingTime =
            p[index].turnaroundTime -
            p[index].burstTime;

        p[index].responseTime =
            startTime - p[index].arrivalTime;

        done[index] = true;

        completed++;
    }

    printResults(p, "SJF");

    return p;
}

vector<Process> priorityScheduling(
    vector<Process> p,
    vector<Segment>& gantt) {

    int n = p.size();

    vector<bool> done(n, false);

    int completed = 0;
    int currentTime = 0;

    while (completed < n) {

        int index = -1;
        int bestPriority = INT_MAX;

        for (int i = 0; i < n; i++) {

            if (!done[i] &&
                p[i].arrivalTime <= currentTime) {

                if (p[i].priority < bestPriority) {

                    bestPriority = p[i].priority;
                    index = i;

                } else if (
                    p[i].priority == bestPriority &&
                    p[i].arrivalTime < p[index].arrivalTime) {

                    index = i;
                }
            }
        }

        if (index == -1) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {

                if (!done[i])
                    nextArrival =
                        min(nextArrival, p[i].arrivalTime);
            }

            addSegment(
                gantt,
                "IDLE",
                currentTime,
                nextArrival
            );

            currentTime = nextArrival;

            continue;
        }

        int startTime = currentTime;

        currentTime += p[index].burstTime;

        addSegment(
            gantt,
            p[index].pid,
            startTime,
            currentTime
        );

        p[index].completionTime = currentTime;

        p[index].turnaroundTime =
            currentTime - p[index].arrivalTime;

        p[index].waitingTime =
            p[index].turnaroundTime -
            p[index].burstTime;

        p[index].responseTime =
            startTime - p[index].arrivalTime;

        done[index] = true;

        completed++;
    }

    printResults(
        p,
        "Priority Scheduling"
    );

    return p;
}

vector<Process> roundRobin(
    vector<Process> p,
    int quantum,
    vector<Segment>& gantt) {

    int n = p.size();

    for (auto& x : p) {

        x.remainingTime = x.burstTime;
        x.responseTime = -1;
    }

    vector<int> order(n);

    for (int i = 0; i < n; i++)
        order[i] = i;

    sort(order.begin(), order.end(),
         [&](int a, int b) {

             if (p[a].arrivalTime ==
                 p[b].arrivalTime)

                 return p[a].pid < p[b].pid;

             return p[a].arrivalTime <
                    p[b].arrivalTime;
         });

    queue<int> readyQueue;

    int currentTime = 0;
    int completed = 0;
    int next = 0;

    while (completed < n) {

        while (
            next < n &&
            p[order[next]].arrivalTime <= currentTime) {

            readyQueue.push(order[next]);

            next++;
        }

        if (readyQueue.empty()) {

            if (next < n) {

                int nextArrival =
                    p[order[next]].arrivalTime;

                addSegment(
                    gantt,
                    "IDLE",
                    currentTime,
                    nextArrival
                );

                currentTime = nextArrival;

                continue;
            }
        }

        int index = readyQueue.front();

        readyQueue.pop();

        if (p[index].responseTime == -1) {

            p[index].responseTime =
                currentTime -
                p[index].arrivalTime;
        }

        int startTime = currentTime;

        int executionTime =
            min(quantum,
                p[index].remainingTime);

        currentTime += executionTime;

        p[index].remainingTime -= executionTime;

        addSegment(
            gantt,
            p[index].pid,
            startTime,
            currentTime
        );

        while (
            next < n &&
            p[order[next]].arrivalTime <= currentTime) {

            readyQueue.push(order[next]);

            next++;
        }

        if (p[index].remainingTime > 0) {

            readyQueue.push(index);

        } else {

            p[index].completionTime =
                currentTime;

            p[index].turnaroundTime =
                p[index].completionTime -
                p[index].arrivalTime;

            p[index].waitingTime =
                p[index].turnaroundTime -
                p[index].burstTime;

            completed++;
        }
    }

    printResults(
        p,
        "Round Robin"
    );

    return p;
}

void displayGanttChart(
    const vector<Segment>& gantt) {

    if (gantt.empty())
        return;

    cout << "\n\n";

    cout << "==================== GANTT CHART ====================\n\n";

    cout << "+";

    for (const auto& segment : gantt) {

        int width =
            max(7,
                (int)segment.pid.length() + 4);

        for (int i = 0; i < width; i++)
            cout << "-";

        cout << "+";
    }

    cout << "\n";

    cout << "|";

    for (const auto& segment : gantt) {

        int width =
            max(7,
                (int)segment.pid.length() + 4);

        int leftSpace =
            (width - segment.pid.length()) / 2;

        int rightSpace =
            width -
            leftSpace -
            segment.pid.length();

        for (int i = 0; i < leftSpace; i++)
            cout << " ";

        cout << segment.pid;

        for (int i = 0; i < rightSpace; i++)
            cout << " ";

        cout << "|";
    }

    cout << "\n";

    cout << "+";

    for (const auto& segment : gantt) {

        int width =
            max(7,
                (int)segment.pid.length() + 4);

        for (int i = 0; i < width; i++)
            cout << "-";

        cout << "+";
    }

    cout << "\n";

    cout << gantt[0].startTime;

    for (const auto& segment : gantt) {

        int width =
            max(7,
                (int)segment.pid.length() + 4);

        cout << setw(width + 1)
             << segment.endTime;
    }

    cout << "\n";
}

void displayComparison(
    const Summary& fcfsSummary,
    const Summary& sjfSummary,
    const Summary& prioritySummary,
    const Summary& rrSummary) {

    cout << "\n\n";

    cout << "============================================================\n";
    cout << "                 ALGORITHM COMPARISON\n";
    cout << "============================================================\n";

    cout << left
         << setw(22) << "Algorithm"
         << setw(15) << "Avg WT"
         << setw(15) << "Avg TAT"
         << setw(15) << "Avg RT"
         << "\n";

    cout << "------------------------------------------------------------\n";

    cout << left
         << setw(22) << "FCFS"
         << setw(15) << fixed << setprecision(2)
         << fcfsSummary.avgWT
         << setw(15)
         << fcfsSummary.avgTAT
         << setw(15)
         << fcfsSummary.avgRT
         << "\n";

    cout << left
         << setw(22) << "SJF"
         << setw(15)
         << sjfSummary.avgWT
         << setw(15)
         << sjfSummary.avgTAT
         << setw(15)
         << sjfSummary.avgRT
         << "\n";

    cout << left
         << setw(22) << "Priority"
         << setw(15)
         << prioritySummary.avgWT
         << setw(15)
         << prioritySummary.avgTAT
         << setw(15)
         << prioritySummary.avgRT
         << "\n";

    cout << left
         << setw(22) << "Round Robin"
         << setw(15)
         << rrSummary.avgWT
         << setw(15)
         << rrSummary.avgTAT
         << setw(15)
         << rrSummary.avgRT
         << "\n";

    cout << "------------------------------------------------------------\n";
}

int main() {

    cout << "\n";

    cout << "============================================================\n";
    cout << "             CPU SCHEDULING SIMULATOR\n";
    cout << "============================================================\n";

    int n;

    cout << "\nEnter number of processes: ";
    cin >> n;

    if (n <= 0) {

        cout << "Invalid number of processes.\n";

        return 0;
    }

    vector<Process> processes(n);

    cout << "\nEnter process details:\n";

    cout << "PID ArrivalTime BurstTime Priority\n\n";

    for (int i = 0; i < n; i++) {

        cin >> processes[i].pid
            >> processes[i].arrivalTime
            >> processes[i].burstTime
            >> processes[i].priority;

        if (
            processes[i].arrivalTime < 0 ||
            processes[i].burstTime <= 0 ||
            processes[i].priority <= 0) {

            cout << "Invalid process data.\n";

            return 0;
        }
    }

    cout << "\n============================================================\n";
    cout << "Select CPU Scheduling Algorithm\n";
    cout << "============================================================\n";

    cout << "1. FCFS\n";
    cout << "2. SJF\n";
    cout << "3. Priority Scheduling\n";
    cout << "4. Round Robin\n";
    cout << "5. Run All Algorithms + Comparison\n";

    int choice;

    cout << "\nEnter your choice: ";
    cin >> choice;

    vector<Segment> gantt;

    if (choice == 1) {

        fcfs(processes, gantt);

        displayGanttChart(gantt);

    } else if (choice == 2) {

        sjf(processes, gantt);

        displayGanttChart(gantt);

    } else if (choice == 3) {

        priorityScheduling(
            processes,
            gantt
        );

        displayGanttChart(gantt);

    } else if (choice == 4) {

        int quantum;

        cout << "\nEnter Time Quantum: ";
        cin >> quantum;

        if (quantum <= 0) {

            cout << "Invalid time quantum.\n";

            return 0;
        }

        roundRobin(
            processes,
            quantum,
            gantt
        );

        displayGanttChart(gantt);

    } else if (choice == 5) {

        int quantum;

        cout << "\nEnter Time Quantum for Round Robin: ";
        cin >> quantum;

        if (quantum <= 0) {

            cout << "Invalid time quantum.\n";

            return 0;
        }

        cout << "\n\nRunning FCFS...\n";

        vector<Segment> fcfsGantt;

        vector<Process> fcfsResult =
            fcfs(
                processes,
                fcfsGantt
            );

        displayGanttChart(fcfsGantt);

        cout << "\n\nRunning SJF...\n";

        vector<Segment> sjfGantt;

        vector<Process> sjfResult =
            sjf(
                processes,
                sjfGantt
            );

        displayGanttChart(sjfGantt);

        cout << "\n\nRunning Priority Scheduling...\n";

        vector<Segment> priorityGantt;

        vector<Process> priorityResult =
            priorityScheduling(
                processes,
                priorityGantt
            );

        displayGanttChart(priorityGantt);

        cout << "\n\nRunning Round Robin...\n";

        vector<Segment> rrGantt;

        vector<Process> rrResult =
            roundRobin(
                processes,
                quantum,
                rrGantt
            );

        displayGanttChart(rrGantt);

        Summary fcfsSummary =
            calculateSummary(fcfsResult);

        Summary sjfSummary =
            calculateSummary(sjfResult);

        Summary prioritySummary =
            calculateSummary(priorityResult);

        Summary rrSummary =
            calculateSummary(rrResult);

        displayComparison(
            fcfsSummary,
            sjfSummary,
            prioritySummary,
            rrSummary
        );

    } else {

        cout << "Invalid choice.\n";

        return 0;
    }

    cout << "\n============================================================\n";
    cout << "             SIMULATION COMPLETED\n";
    cout << "============================================================\n";

    return 0;
}
