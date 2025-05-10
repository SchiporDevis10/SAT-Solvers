#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <sstream>
#include <chrono>
using namespace std;

bool unitPropagation(set<set<int>>& clauses, set<int>& assignment) {
    bool changed = true;
    while (changed) {
        changed = false;
        set<set<int>> newClauses;
        for (auto& clause : clauses) {
            if (clause.size() == 1) {
                int unit = *clause.begin();
                if (assignment.count(-unit)) return false;
                assignment.insert(unit);
                changed = true;

                for (auto& c : clauses) {
                    if (c.count(unit)) continue;
                    if (c.count(-unit)) {
                        set<int> reduced = c;
                        reduced.erase(-unit);
                        if (reduced.empty()) return false;
                        newClauses.insert(reduced);
                    } else {
                        newClauses.insert(c);
                    }
                }
                clauses = newClauses;
                break;
            }
        }
    }
    return true;
}

void pureLiteralElimination(set<set<int>>& clauses, set<int>& assignment, vector<int>& freqPos, vector<int>& freqNeg) {
    set<int> pures;
    for (int i = 1; i < freqPos.size(); i++) {
        if ((freqPos[i] > 0 && freqNeg[i] == 0)) pures.insert(i);
        if ((freqNeg[i] > 0 && freqPos[i] == 0)) pures.insert(-i);
    }
    if (pures.empty()) return;

    set<set<int>> newClauses;
    for (auto& clause : clauses) {
        bool hasPure = false;
        for (int lit : clause) {
            if (pures.count(lit)) {
                hasPure = true;
                assignment.insert(lit);
                break;
            }
        }
        if (!hasPure) newClauses.insert(clause);
    }
    clauses = newClauses;
}

bool DPLL(set<set<int>> clauses, set<int>& assignment, vector<int> freqPos, vector<int> freqNeg) {
    if (!unitPropagation(clauses, assignment)) return false;
    if (clauses.empty()) return true;
    pureLiteralElimination(clauses, assignment, freqPos, freqNeg);
    if (clauses.empty()) return true;

    for (auto& clause : clauses) {
        for (int lit : clause) {
            set<int> assignTrue = assignment, assignFalse = assignment;
            set<set<int>> clausesTrue = clauses, clausesFalse = clauses;
            vector<int> freqPosT = freqPos, freqNegT = freqNeg;
            vector<int> freqPosF = freqPos, freqNegF = freqNeg;

            if (lit > 0) freqPosT[lit]++, freqNegF[lit]++;
            else freqNegT[-lit]++, freqPosF[-lit]++;

            clausesTrue.insert({lit});
            clausesFalse.insert({-lit});

            if (DPLL(clausesTrue, assignTrue, freqPosT, freqNegT)) {
                assignment = assignTrue;
                return true;
            }
            if (DPLL(clausesFalse, assignFalse, freqPosF, freqNegF)) {
                assignment = assignFalse;
                return true;
            }

            return false;
        }
    }
    return false;
}

int main() {
    ifstream fin("formula.cnf");
    if (!fin) {
        cerr << "Eroare la deschiderea fisierului.\n";
        return 1;
    }

    string line;
    int numVars, numClauses;
    set<set<int>> clauses;
    vector<int> freqPos(1001, 0), freqNeg(1001, 0);

    while (getline(fin, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') {
            sscanf(line.c_str(), "p cnf %d %d", &numVars, &numClauses);
            freqPos.resize(numVars + 1);
            freqNeg.resize(numVars + 1);
            continue;
        }
        if (line[0] == '%') break;
        int lit;
        set<int> clause;
        istringstream iss(line);
        while (iss >> lit && lit != 0) {
            clause.insert(lit);
            if (lit > 0) freqPos[lit]++;
            else freqNeg[-lit]++;
        }
        clauses.insert(clause);
    }
    fin.close();

    set<int> assignment;
    auto start = std::chrono::high_resolution_clock::now();
    bool result = DPLL(clauses, assignment, freqPos, freqNeg);

    if (result) {
        cout << "SATISFIABILA" << endl;
        vector<int> sortedAssignment(assignment.begin(), assignment.end());
        sort(sortedAssignment.begin(), sortedAssignment.end(), [](int a, int b) {
            return abs(a) < abs(b);
        });
        cout << "Valori adevarate: ";
        for (int lit : sortedAssignment) {
            cout << lit << " ";
        }
        cout << endl;
    } else {
        cout << "NESATISFIABILA" << endl;
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    cout << "\nTimpul de executie: " << duration.count() / 1000000000.0 << " secunde \n";
    return 0;
}
