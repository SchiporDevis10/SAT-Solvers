#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <set>
#include <chrono>
using namespace std;

// Functia pentru compararea clauzelor (ordoneaza clauzele lexicografic)
bool compareClauses(const vector<int>& c1, const vector<int>& c2) {
    return c1 < c2;
}

void pureLiteralElimination(
    vector<vector<int>>& clauses,
    set<vector<int>, bool(*)(const vector<int>&, const vector<int>&)>& clauseSet,
    const set<int>& pos,
    const set<int>& neg
) {
    // Identificam literalii puri
    set<int> pureLiterals;
    for (int lit : pos) {
        if (neg.find(lit) == neg.end()) {
            pureLiterals.insert(lit); // doar pozitiv
        }
    }
    for (int lit : neg) {
        if (pos.find(lit) == pos.end()) {
            pureLiterals.insert(-lit); // doar negativ
        }
    }

    if (pureLiterals.empty()) return;

    vector<vector<int>> newClauses;
    for (const auto& clause : clauses) {
        bool containsPure = false;
        for (int lit : clause) {
            if (pureLiterals.find(lit) != pureLiterals.end()) {
                containsPure = true;
                break;
            }
        }
        if (!containsPure) {
            newClauses.push_back(clause);  // pastram doar clauzele fara literali puri
        }
    }

    // Refacem setul cu clauze
    clauseSet.clear();
    for (const auto& clause : newClauses)
        clauseSet.insert(clause);

    clauses = newClauses;
}


bool unitPropagation(vector<vector<int>>& clauses, set<vector<int>, decltype(&compareClauses)>& clauseSet) {
    bool changed = false;
    set<int> assignedLiterals;

    while (true) {
        int unitLiteral = 0;
        for (const auto& clause : clauses) {
            if (clause.size() == 1) {
                unitLiteral = clause[0];
                break;
            }
        }

        if (unitLiteral == 0) break; // Nu mai sunt clauze unitare

        changed = true;
        assignedLiterals.insert(unitLiteral);

        vector<vector<int>> newClauses;
        for (auto& clause : clauses) {
            if (find(clause.begin(), clause.end(), unitLiteral) != clause.end()) {
                continue; // Clauza este satisfacuta, o eliminam
            }

            if (find(clause.begin(), clause.end(), -unitLiteral) != clause.end()) {
                vector<int> newClause;
                for (int lit : clause) {
                    if (lit != -unitLiteral) {
                        newClause.push_back(lit);
                    }
                }

                if (newClause.empty()) {
                    return false; // S-a generat clauza vida: contradicție
                }

                sort(newClause.begin(), newClause.end());
                if (clauseSet.find(newClause) == clauseSet.end()) {
                    newClauses.push_back(newClause);
                    clauseSet.insert(newClause);
                }
            } else {
                newClauses.push_back(clause); // Clauza ramane nemodificata
            }
        }

        clauses = newClauses;
    }

    return true; // Nu s-a intalnit contradicție
}




// Clauze triviale
bool isTrivialClause(const vector<int>& clause) {
    set<int> seen;
    for (int lit : clause) {
        if (seen.count(-lit)) return true;
        seen.insert(lit);
    }
    return false;
}

vector<vector<int>> removeTrivialClauses(const vector<vector<int>>& inputClauses) {
    vector<vector<int>> filtered;
    for (const auto& clause : inputClauses) {
        if (!isTrivialClause(clause)) {
            filtered.push_back(clause);
        }
    }
    return filtered;
}

// Functia resolvent
vector<int> resolvent(const vector<int>& c1, const vector<int>& c2) {
    int oppCount = 0; // Perechi opuse
    int oppLiteral = 0; // Literalul opus
    set<int> s1(c1.begin(), c1.end());
    set<int> s2(c2.begin(), c2.end());

    // Cautam o pereche de literali opusi
    for (int lit : s1) {
        if (s2.count(-lit)) {
            oppCount++;
            oppLiteral = lit;
        }
    }

    if (oppCount != 1) return {};  // Returneaza un vector gol daca nu exista rezolvent

    set<int> resolventSet;
    for (int lit : c1)
        if (lit != oppLiteral)
            resolventSet.insert(lit);
    for (int lit : c2)
        if (lit != -oppLiteral)
            resolventSet.insert(lit);

    if (resolventSet.empty()) {
        return {0};  // Clauza vida
    }

    return vector<int>(resolventSet.begin(), resolventSet.end());
}

int main() {
    ifstream fin("formula.cnf");
    if (!fin.is_open()) {
        cerr << "Fisierul nu a putut fi deschis.\n";
        return 1;
    }

    string line;
    int numVariables = 0, numClauses = 0;
    vector<vector<int>> clauses;
    set<vector<int>, decltype(&compareClauses)> clauseSet(compareClauses);  // Set cu comparator pentru ordonare lexicografica
    //set<vector<int>> clauseSet; // Set fara comparator
    set<int> pos, neg;  // Seturi pentru literalii pozitivi și negativi - util la eliminarea literalului pur

    // Citirea fisierului CNF
    while (getline(fin, line)) {
        if (line.empty()) continue;
        if (line[0] == 'c') continue;
        if (line[0] == 'p') {
            istringstream header(line);
            string temp;
            header >> temp >> temp >> numVariables >> numClauses;
            continue;
        }
        istringstream iss(line);
        int literal;
        vector<int> clause;
        while (iss >> literal && literal != 0) {
            clause.push_back(literal);

            if (literal > 0) pos.insert(literal);
            else neg.insert(-literal);
        }
        if (!clause.empty()) {
            sort(clause.begin(), clause.end());
            clauseSet.insert(clause); // Clauzele vor fi automat ordonate
            clauses.push_back(clause);
        }
    }
    fin.close();
    clauses = removeTrivialClauses(clauses);
    clauseSet.clear();
    for (const auto& clause : clauses) {
        clauseSet.insert(clause);  // Refacem setul fara clauze triviale
    }
    // Comentam secțiunea de afișare a clauzelor citite

    cout << "Clauzele citite:\n";
    for (const auto& clauza : clauseSet) {
        for (int lit : clauza) {
            cout << lit << ' ';
        }
        cout << endl;
    }

    auto start = std::chrono::high_resolution_clock::now();

    // Aplicarea DP
    bool contradiction = false;
    int clauseCount = 0;
    int maxClauses = 10000;
    vector<vector<int>> lastClauses = clauses;  // Ultimele clauze adaugate

    // Continuam pana cand nu mai exista noi clauze de adaugat sau gasim o contradictie
    while (clauseCount < maxClauses) {

        if (!unitPropagation(clauses, clauseSet)) {
            contradiction = true;
            break;
        }

        pureLiteralElimination(clauses, clauseSet, pos, neg);

        vector<vector<int>> newClauses;

        for (size_t i = 0; i < clauses.size(); ++i) {
            for (size_t j = i + 1; j < clauses.size(); ++j) {
                vector<int> res = resolvent(clauses[i], clauses[j]);
                if (res.empty()) {
                    continue;
                }

                // Verificam daca exista o contradicție (adica o clauza vida, {0})
                if (res == vector<int>{0}) {
                    contradiction = true;
                    break;
                }

                // Sortam rezolventa pentru a evita dublurile
                sort(res.begin(), res.end());

                if (isTrivialClause(res)) {
                continue; // Sarim peste adaugarea clauzei triviale
                }

                // Inseram clauza doar daca nu exista deja
                if (clauseSet.find(res) == clauseSet.end()) {
                    clauseSet.insert(res);
                    newClauses.push_back(res);
                    clauseCount ++;  // Incrementam numarul de clauze
                }
            }
            if (contradiction) break;
        }

        if (contradiction || newClauses.empty()) break;

        for (const auto& c : newClauses)
            clauses.push_back(c);

        // Verificam daca formula a ajuns intr-o stare stabila (adica nu au mai aparut clauze noi)
        if (newClauses == lastClauses) {
            break;
        }

        lastClauses = newClauses;  // Actualizam ultima stare a clauzelor
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    cout << "Timpul de executie: " << duration.count() / 1000000000.0 << "\n";
    cout << "\nFormula este " << (contradiction ? "NESATISFIABILA" : "SATISFIABILA") << ".\n";
    return 0;
}
