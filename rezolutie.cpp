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

    // Cautam o pereche de literali opuse
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

    // Citirea fisierului formula.cnf
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
        while (iss >> literal && literal != 0)
            clause.push_back(literal);

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

    cout << "Clauzele citite:\n";
    for (const auto& clause : clauses) {
        for (int lit : clause) {
            cout << lit << ' ';
        }
        cout << "\n";
    }

    auto start = std::chrono::high_resolution_clock::now();
    // Aplicarea rezolutiei
    bool contradiction = false;
    int iteration = 0;
    int clauseCount = 0;
    int maxIterations = 10000;
    int maxClauses = 10000;
    vector<vector<int>> lastClauses = clauses;  // Ultimele clauze adaugate

    // Continuam pana cand nu mai exista noi clauze de adaugat sau gasim o contradictie
    while (iteration < maxIterations && clauseCount < maxClauses) {
        vector<vector<int>> newClauses;

        for (size_t i = 0; i < clauses.size(); ++i) {
            for (size_t j = i + 1; j < clauses.size(); ++j) {
                vector<int> res = resolvent(clauses[i], clauses[j]);
                if (res.empty()) {
                    continue;
                }

                // Verificam dacă exista o contradictie (adica o clauza vida, {0})
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
    cout << "Timpul de executie: " << duration.count() / 1000000000.0 << " secunde \n";
    cout << "\nFormula este " << (contradiction ? "NESATISFIABILA" : "SATISFIABILA") << ".\n";
    return 0;
}
