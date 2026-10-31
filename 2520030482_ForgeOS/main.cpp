#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <string>
#include <map>
#include <climits>

using namespace std;

struct Process {
    string pid;
    int arrivalTime;
    int burstTime;
    int priority;
    int queue;
    int tickets;
    string userGroup;
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



vector<Process> srtf(vector<Process> p, vector<Segment>& gantt) {

    int n = p.size();

    for (auto& x : p) {
        x.remainingTime = x.burstTime;
        x.responseTime = -1;
    }

    int completed = 0;
    int currentTime = 0;

    while (completed < n) {

        int selected = -1;
        int shortestRemaining = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrivalTime <= currentTime &&
                p[i].remainingTime > 0 &&
                p[i].remainingTime < shortestRemaining) {

                shortestRemaining = p[i].remainingTime;
                selected = i;
            }
        }

        if (selected == -1) {
            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {
                if (p[i].remainingTime > 0)
                    nextArrival = min(nextArrival, p[i].arrivalTime);
            }

            currentTime = max(currentTime, nextArrival);
            continue;
        }

        if (p[selected].responseTime == -1)
            p[selected].responseTime =
                currentTime - p[selected].arrivalTime;

        int start = currentTime;

        p[selected].remainingTime--;
        currentTime++;

        addSegment(gantt, p[selected].pid, start, currentTime);

        if (p[selected].remainingTime == 0) {

            p[selected].completionTime = currentTime;

            p[selected].turnaroundTime =
                p[selected].completionTime - p[selected].arrivalTime;

            p[selected].waitingTime =
                p[selected].turnaroundTime - p[selected].burstTime;

            completed++;
        }
    }

    return p;
}


vector<Process> hrrn(vector<Process> p, vector<Segment>& gantt) {

    int n = p.size();
    int completed = 0;
    int currentTime = 0;

    vector<bool> done(n, false);

    while (completed < n) {

        int selected = -1;
        double highestRatio = -1.0;

        for (int i = 0; i < n; i++) {

            if (!done[i] && p[i].arrivalTime <= currentTime) {

                int waiting =
                    currentTime - p[i].arrivalTime;

                double ratio =
                    (double)(waiting + p[i].burstTime)
                    / p[i].burstTime;

                if (ratio > highestRatio) {
                    highestRatio = ratio;
                    selected = i;
                }
            }
        }

        if (selected == -1) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {
                if (!done[i])
                    nextArrival =
                        min(nextArrival, p[i].arrivalTime);
            }

            currentTime = max(currentTime, nextArrival);
            continue;
        }

        int start = currentTime;

        p[selected].responseTime =
            currentTime - p[selected].arrivalTime;

        currentTime += p[selected].burstTime;

        p[selected].completionTime = currentTime;

        p[selected].turnaroundTime =
            p[selected].completionTime -
            p[selected].arrivalTime;

        p[selected].waitingTime =
            p[selected].turnaroundTime -
            p[selected].burstTime;

        addSegment(
            gantt,
            p[selected].pid,
            start,
            currentTime
        );

        done[selected] = true;
        completed++;
    }

    return p;
}


vector<Process> multilevelQueue(vector<Process> p, vector<Segment>& gantt) {

    int n = p.size();
    int completed = 0;
    int currentTime = 0;

    vector<bool> done(n, false);

    while (completed < n) {

        int selected = -1;
        int bestQueue = INT_MAX;

        for (int i = 0; i < n; i++) {

            if (!done[i] &&
                p[i].arrivalTime <= currentTime) {

                if (p[i].queue < bestQueue) {
                    bestQueue = p[i].queue;
                    selected = i;
                }
            }
        }

        if (selected == -1) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {
                if (!done[i])
                    nextArrival =
                        min(nextArrival, p[i].arrivalTime);
            }

            currentTime = max(currentTime, nextArrival);
            continue;
        }

        int start = currentTime;

        p[selected].responseTime =
            currentTime - p[selected].arrivalTime;

        currentTime += p[selected].burstTime;

        p[selected].completionTime = currentTime;

        p[selected].turnaroundTime =
            p[selected].completionTime -
            p[selected].arrivalTime;

        p[selected].waitingTime =
            p[selected].turnaroundTime -
            p[selected].burstTime;

        addSegment(
            gantt,
            p[selected].pid,
            start,
            currentTime
        );

        done[selected] = true;
        completed++;
    }

    return p;
}


vector<Process> mlfq(vector<Process> p, vector<Segment>& gantt) {

    int n = p.size();

    for (auto& x : p) {
        x.remainingTime = x.burstTime;
        x.responseTime = -1;
    }

    vector<int> level(n, 0);
    vector<bool> completed(n, false);

    int finished = 0;
    int currentTime = 0;

    const int quantum[3] = {2, 4, 8};

    while (finished < n) {

        int selected = -1;
        int bestLevel = 3;

        for (int i = 0; i < n; i++) {

            if (!completed[i] &&
                p[i].arrivalTime <= currentTime &&
                level[i] < bestLevel) {

                selected = i;
                bestLevel = level[i];
            }
        }

        if (selected == -1) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {
                if (!completed[i])
                    nextArrival =
                        min(nextArrival, p[i].arrivalTime);
            }

            currentTime = max(currentTime, nextArrival);
            continue;
        }

        if (p[selected].responseTime == -1)
            p[selected].responseTime =
                currentTime - p[selected].arrivalTime;

        int q = quantum[level[selected]];
        int runTime = min(q, p[selected].remainingTime);

        int start = currentTime;

        currentTime += runTime;
        p[selected].remainingTime -= runTime;

        addSegment(
            gantt,
            p[selected].pid,
            start,
            currentTime
        );

        if (p[selected].remainingTime == 0) {

            p[selected].completionTime = currentTime;

            p[selected].turnaroundTime =
                p[selected].completionTime -
                p[selected].arrivalTime;

            p[selected].waitingTime =
                p[selected].turnaroundTime -
                p[selected].burstTime;

            completed[selected] = true;
            finished++;

        } else if (level[selected] < 2) {

            level[selected]++;
        }
    }

    return p;
}


vector<Process> fcfsAging(vector<Process> p, vector<Segment>& gantt) {

    int n = p.size();
    int completed = 0;
    int currentTime = 0;

    vector<bool> done(n, false);

    while (completed < n) {

        int selected = -1;
        int bestAgingPriority = INT_MIN;

        for (int i = 0; i < n; i++) {

            if (!done[i] &&
                p[i].arrivalTime <= currentTime) {

                int waiting =
                    currentTime - p[i].arrivalTime;

                int agingPriority =
                    p[i].priority + (waiting / 5);

                if (selected == -1 ||
                    agingPriority > bestAgingPriority ||
                    (agingPriority == bestAgingPriority &&
                     p[i].arrivalTime < p[selected].arrivalTime)) {

                    selected = i;
                    bestAgingPriority = agingPriority;
                }
            }
        }

        if (selected == -1) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {
                if (!done[i])
                    nextArrival =
                        min(nextArrival, p[i].arrivalTime);
            }

            currentTime = max(currentTime, nextArrival);
            continue;
        }

        int start = currentTime;

        p[selected].responseTime =
            currentTime - p[selected].arrivalTime;

        currentTime += p[selected].burstTime;

        p[selected].completionTime = currentTime;

        p[selected].turnaroundTime =
            p[selected].completionTime -
            p[selected].arrivalTime;

        p[selected].waitingTime =
            p[selected].turnaroundTime -
            p[selected].burstTime;

        addSegment(
            gantt,
            p[selected].pid,
            start,
            currentTime
        );

        done[selected] = true;
        completed++;
    }

    return p;
}


vector<Process> priorityAging(vector<Process> p, vector<Segment>& gantt) {

    int n = p.size();
    int completed = 0;
    int currentTime = 0;

    vector<bool> done(n, false);

    while (completed < n) {

        int selected = -1;
        int bestPriority = INT_MAX;

        for (int i = 0; i < n; i++) {

            if (!done[i] &&
                p[i].arrivalTime <= currentTime) {

                int waiting =
                    currentTime - p[i].arrivalTime;

                int effectivePriority =
                    p[i].priority - (waiting / 5);

                if (selected == -1 ||
                    effectivePriority < bestPriority ||
                    (effectivePriority == bestPriority &&
                     p[i].arrivalTime < p[selected].arrivalTime)) {

                    selected = i;
                    bestPriority = effectivePriority;
                }
            }
        }

        if (selected == -1) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {
                if (!done[i])
                    nextArrival =
                        min(nextArrival, p[i].arrivalTime);
            }

            currentTime = max(currentTime, nextArrival);
            continue;
        }

        int start = currentTime;

        p[selected].responseTime =
            currentTime - p[selected].arrivalTime;

        currentTime += p[selected].burstTime;

        p[selected].completionTime = currentTime;

        p[selected].turnaroundTime =
            p[selected].completionTime -
            p[selected].arrivalTime;

        p[selected].waitingTime =
            p[selected].turnaroundTime -
            p[selected].burstTime;

        addSegment(
            gantt,
            p[selected].pid,
            start,
            currentTime
        );

        done[selected] = true;
        completed++;
    }

    return p;
}


vector<Process> lotteryScheduling(vector<Process> p, vector<Segment>& gantt) {

    int n = p.size();
    int completed = 0;
    int currentTime = 0;

    vector<bool> done(n, false);

    while (completed < n) {

        vector<int> ready;

        for (int i = 0; i < n; i++) {
            if (!done[i] &&
                p[i].arrivalTime <= currentTime) {
                ready.push_back(i);
            }
        }

        if (ready.empty()) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {
                if (!done[i])
                    nextArrival =
                        min(nextArrival, p[i].arrivalTime);
            }

            currentTime = max(currentTime, nextArrival);
            continue;
        }

        int totalTickets = 0;

        for (int i : ready) {
            totalTickets += max(1, p[i].tickets);
        }

        int winningTicket =
            (rand() % totalTickets) + 1;

        int cumulative = 0;
        int selected = ready[0];

        for (int i : ready) {

            cumulative += max(1, p[i].tickets);

            if (winningTicket <= cumulative) {
                selected = i;
                break;
            }
        }

        int start = currentTime;

        p[selected].responseTime =
            currentTime - p[selected].arrivalTime;

        currentTime += p[selected].burstTime;

        p[selected].completionTime = currentTime;

        p[selected].turnaroundTime =
            p[selected].completionTime -
            p[selected].arrivalTime;

        p[selected].waitingTime =
            p[selected].turnaroundTime -
            p[selected].burstTime;

        addSegment(
            gantt,
            p[selected].pid,
            start,
            currentTime
        );

        done[selected] = true;
        completed++;
    }

    return p;
}


vector<Process> fairShareScheduling(vector<Process> p, vector<Segment>& gantt) {

    int n = p.size();
    int completed = 0;
    int currentTime = 0;

    vector<bool> done(n, false);

    while (completed < n) {

        vector<int> ready;

        for (int i = 0; i < n; i++) {
            if (!done[i] &&
                p[i].arrivalTime <= currentTime) {
                ready.push_back(i);
            }
        }

        if (ready.empty()) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {
                if (!done[i])
                    nextArrival =
                        min(nextArrival, p[i].arrivalTime);
            }

            currentTime = max(currentTime, nextArrival);
            continue;
        }

        map<string, int> groupUsage;

        for (int i : ready) {
            groupUsage[p[i].userGroup]++;
        }

        int selected = ready[0];

        for (int i : ready) {

            string currentGroup =
                p[selected].userGroup;

            string candidateGroup =
                p[i].userGroup;

            if (groupUsage[candidateGroup] <
                groupUsage[currentGroup]) {

                selected = i;
            }
        }

        int start = currentTime;

        p[selected].responseTime =
            currentTime - p[selected].arrivalTime;

        currentTime += p[selected].burstTime;

        p[selected].completionTime = currentTime;

        p[selected].turnaroundTime =
            p[selected].completionTime -
            p[selected].arrivalTime;

        p[selected].waitingTime =
            p[selected].turnaroundTime -
            p[selected].burstTime;

        addSegment(
            gantt,
            p[selected].pid,
            start,
            currentTime
        );

        done[selected] = true;
        completed++;
    }

    return p;
}


vector<Process> hybridMLQRR(
    vector<Process> p,
    int quantum,
    vector<Segment>& gantt) {

    int n = p.size();

    for (auto& x : p) {
        x.remainingTime = x.burstTime;
        x.responseTime = -1;
    }

    int completed = 0;
    int currentTime = 0;

    while (completed < n) {

        int selectedQueue = INT_MAX;

        for (const auto& x : p) {
            if (x.remainingTime > 0 &&
                x.arrivalTime <= currentTime) {

                selectedQueue =
                    min(selectedQueue, x.queue);
            }
        }

        if (selectedQueue == INT_MAX) {

            int nextArrival = INT_MAX;

            for (const auto& x : p) {
                if (x.remainingTime > 0)
                    nextArrival =
                        min(nextArrival, x.arrivalTime);
            }

            currentTime =
                max(currentTime, nextArrival);

            continue;
        }

        vector<int> ready;

        for (int i = 0; i < n; i++) {
            if (p[i].remainingTime > 0 &&
                p[i].arrivalTime <= currentTime &&
                p[i].queue == selectedQueue) {

                ready.push_back(i);
            }
        }

        if (ready.empty())
            continue;

        for (int selected : ready) {

            if (p[selected].remainingTime <= 0)
                continue;

            if (p[selected].responseTime == -1)
                p[selected].responseTime =
                    currentTime - p[selected].arrivalTime;

            int runTime =
                min(quantum, p[selected].remainingTime);

            int start = currentTime;

            currentTime += runTime;

            p[selected].remainingTime -= runTime;

            addSegment(
                gantt,
                p[selected].pid,
                start,
                currentTime
            );

            if (p[selected].remainingTime == 0) {

                p[selected].completionTime =
                    currentTime;

                p[selected].turnaroundTime =
                    p[selected].completionTime -
                    p[selected].arrivalTime;

                p[selected].waitingTime =
                    p[selected].turnaroundTime -
                    p[selected].burstTime;

                completed++;
            }

            bool higherQueueArrived = false;

            for (const auto& x : p) {

                if (x.remainingTime > 0 &&
                    x.arrivalTime <= currentTime &&
                    x.queue < selectedQueue) {

                    higherQueueArrived = true;
                    break;
                }
            }

            if (higherQueueArrived)
                break;
        }
    }

    return p;
}

struct Decision {
    string algorithm;
    string reason;
    int quantum;
};

Decision selectAlgorithm(const vector<Process>& p) {

    int n = p.size();
    int differentQueues = 0;
    int differentGroups = 0;
    int minQueue = INT_MAX, maxQueue = INT_MIN;

    vector<int> queues;
    vector<string> groups;

    for (const auto& x : p) {
        queues.push_back(x.queue);
        groups.push_back(x.userGroup);
        minQueue = min(minQueue, x.queue);
        maxQueue = max(maxQueue, x.queue);
    }

    sort(queues.begin(), queues.end());
    queues.erase(unique(queues.begin(), queues.end()), queues.end());
    differentQueues = queues.size();

    sort(groups.begin(), groups.end());
    groups.erase(unique(groups.begin(), groups.end()), groups.end());
    differentGroups = groups.size();

    int minBurst = p[0].burstTime;
    int maxBurst = p[0].burstTime;
    int minPriority = p[0].priority;
    int maxPriority = p[0].priority;

    for (const auto& x : p) {
        minBurst = min(minBurst, x.burstTime);
        maxBurst = max(maxBurst, x.burstTime);
        minPriority = min(minPriority, x.priority);
        maxPriority = max(maxPriority, x.priority);
    }

    /*
       These are project-designed selection rules.
       The faculty provided the nine algorithms, but not
       the automatic-selection criteria.
    */

    if (differentGroups > 1 && differentQueues > 1) {
        return {
            "Multilevel Queue + Round Robin Hybrid",
            "Multiple user groups and multiple queues are present, so a hybrid queue-based policy is suitable.",
            2
        };
    }

    if (differentGroups > 1) {
        return {
            "Fair Share Scheduling",
            "Multiple user groups are present, so CPU time can be distributed fairly among groups.",
            2
        };
    }

    bool variedTickets = false;
    int minTickets = p[0].tickets;
    int maxTickets = p[0].tickets;

    for (const auto& x : p) {
        minTickets = min(minTickets, x.tickets);
        maxTickets = max(maxTickets, x.tickets);
    }

    variedTickets = (maxTickets > minTickets);

    if (variedTickets && differentQueues == 1 && differentGroups == 1) {
        return {
            "Lottery Scheduling",
            "Processes have different ticket counts, so ticket-based CPU selection can be applied.",
            2
        };
    }

    if (differentQueues > 1 && maxBurst - minBurst >= 5) {
        return {
            "Multilevel Feedback Queue (MLFQ)",
            "Multiple queues and significantly different burst times indicate a workload suitable for feedback-based scheduling.",
            2
        };
    }

    if (differentQueues > 1) {
        return {
            "Multilevel Queue Scheduling (MLQ)",
            "Processes are distributed across multiple queues, so queue-based scheduling can be applied.",
            2
        };
    }

    if (maxPriority - minPriority >= 2) {
        return {
            "Priority Scheduling with Aging",
            "There is a noticeable priority difference, so aging can reduce starvation of lower-priority processes.",
            2
        };
    }

    if (maxBurst - minBurst >= 4) {
        return {
            "Shortest Remaining Time First (SRTF)",
            "Burst times vary significantly, so preemptive shortest-remaining-time scheduling is suitable.",
            2
        };
    }

    if (maxBurst - minBurst >= 2) {
        return {
            "Highest Response Ratio Next (HRRN)",
            "Processes have different burst times, making response-ratio-based selection useful.",
            2
        };
    }

    return {
        "First-Come, First-Served with Aging",
        "The workload has relatively similar process characteristics, with aging added to reduce long-term waiting.",
        2
    };
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

    cout << "PID ArrivalTime BurstTime Priority Queue Tickets UserGroup\n\n";

    for (int i = 0; i < n; i++) {

        cin >> processes[i].pid
            >> processes[i].arrivalTime
            >> processes[i].burstTime
            >> processes[i].priority
            >> processes[i].queue
            >> processes[i].tickets
            >> processes[i].userGroup;;

        if (
            processes[i].arrivalTime < 0 ||
            processes[i].burstTime <= 0 ||
            processes[i].priority <= 0) {

            cout << "Invalid process data.\n";

            return 0;
        }
    }

    cout << "\n============================================================\n";
    cout << "        AUTOMATIC CPU SCHEDULING ANALYSIS\n";
    cout << "============================================================\n";

    cout << "Analyzing process characteristics...\n";

    vector<Segment> gantt;

    // Automatic algorithm selection will be added here.
    Decision decision = selectAlgorithm(processes);

    cout << "\nSelected Algorithm : " << decision.algorithm << "\n";
    cout << "Reason              : " << decision.reason << "\n";
    cout << "System-selected Quantum : " << decision.quantum << "\n";


    if (decision.algorithm == "Multilevel Queue + Round Robin Hybrid") {
        cout << "\nRunning selected scheduling algorithm...\n";

        vector<Process> result =
            hybridMLQRR(
                processes,
                decision.quantum,
                gantt
            );

        displayGanttChart(gantt);
        printResults(result, "Multilevel Queue + Round Robin Hybrid");

    } else if (decision.algorithm == "Shortest Remaining Time First (SRTF)") {
        cout << "\nRunning selected scheduling algorithm...\n";

        vector<Process> result = srtf(processes, gantt);

        displayGanttChart(gantt);
        printResults(result, "SRTF");

    } else if (decision.algorithm == "Highest Response Ratio Next (HRRN)") {
        cout << "\nRunning selected scheduling algorithm...\n";

        vector<Process> result = hrrn(processes, gantt);

        displayGanttChart(gantt);
        printResults(result, "HRRN");

    } else if (decision.algorithm == "Multilevel Queue Scheduling (MLQ)") {
        cout << "\nRunning selected scheduling algorithm...\n";

        vector<Process> result =
            multilevelQueue(processes, gantt);

        displayGanttChart(gantt);
        printResults(result, "MLQ");

    } else if (decision.algorithm == "Multilevel Feedback Queue (MLFQ)") {
        cout << "\nRunning selected scheduling algorithm...\n";

        vector<Process> result =
            mlfq(processes, gantt);

        displayGanttChart(gantt);
        printResults(result, "MLFQ");

    } else if (decision.algorithm == "First-Come, First-Served with Aging") {
        cout << "\nRunning selected scheduling algorithm...\n";

        vector<Process> result =
            fcfsAging(processes, gantt);

        displayGanttChart(gantt);
        printResults(result, "FCFS with Aging");

    } else if (decision.algorithm == "Priority Scheduling with Aging") {
        cout << "\nRunning selected scheduling algorithm...\n";

        vector<Process> result =
            priorityAging(processes, gantt);

        displayGanttChart(gantt);
        printResults(result, "Priority Scheduling with Aging");

    } else if (decision.algorithm == "Lottery Scheduling") {
        cout << "\nRunning selected scheduling algorithm...\n";

        vector<Process> result =
            lotteryScheduling(processes, gantt);

        displayGanttChart(gantt);
        printResults(result, "Lottery Scheduling");

    } else if (decision.algorithm == "Fair Share Scheduling") {
        cout << "\nRunning selected scheduling algorithm...\n";

        vector<Process> result =
            fairShareScheduling(processes, gantt);

        displayGanttChart(gantt);
        printResults(result, "Fair Share Scheduling");
    }


    cout << "\n============================================================\n";
    cout << "             SIMULATION COMPLETED\n";
    cout << "============================================================\n";

    return 0;
}
