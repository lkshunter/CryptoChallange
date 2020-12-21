#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <NTL/mat_GF2.h>
#include <algorithm>

using namespace std;

/*
 * Entfernt einen bestimten Char aus einem char Pointer
 * */
void removeChar(char *str, char element) {
    char *dest = str;
    while (*str) {
        if (*str != element) {
            *dest++ = *str;
        }
        str++;
    }
    *dest = '\0';
}

/*
 * Erzeugt aus einem char Pointer
 * */
vector<vector<vector<int>>> createPublicKey(char *pKey) {

    /*
     * Logischer Aufbau des publicKey
     *
     * vector1_PublicKey(
     *      vector2_Funktion_1(
     *          vector3_multplikatio_1(
     *              1,2
     *          ),
	 *		    vector3_multplikatio_2(
	 * 			    3,5
	 *		    ),
	 *		    vector3_multplikatio_3(
	 *			    3
	 *		    ),
	 * 		    vector3_multplikatio_4(
	 *			    4
	 *		    ),
	 *		    ...
	 *		    vector3_multplikatio_n(
	 *			    ...
	 *		    )
	 *      ),
	 *      ...
	 *      vector2_Funktion_n(
	 *	        ...
	 *      )
     *  )
     * */

    /*
     * Entfernt chars die nicht für die weiter verarbeitung benötigt werden
     * */
    removeChar(pKey, '\n');
    removeChar(pKey, ' ');
    removeChar(pKey, '_');
    removeChar(pKey, 'x');

    /*
     * Teilt den String an einem ',' in die verschiedenen Funktionen auf und speichert sie in einen Vector
     * */
    stringstream test(pKey);
    string segment;
    vector<string> seglist;

    while (getline(test, segment, ',')) {
        seglist.push_back(segment);
    }

    /*
     * Teilt jede Funktion in ihre Summanden auf
     * */
    vector<vector<string>> list;
    for (vector<string>::iterator it = seglist.begin(); it != seglist.end(); ++it) {

        stringstream test1(*it);
        string segment;
        vector<string> temp;

        while (getline(test1, segment, '+')) {
            temp.push_back(segment);
        }
        list.push_back(temp);

    }

    /*
     * Die Summanden können aus einer Multiplikation bestehen
     * Für den Fall wird sie aufgeteilt
     *
     * Ob eine Multiplikation oder nur ein Summand sie werden in einem Vector angelegt
     * */
    vector<vector<vector<int>>> key;
    for (int i = 0; i < list.size(); i++) {

        vector<vector<int>> temp2;
        string segment;
        for (vector<string>::iterator it = list.at(i).begin(); it != list.at(i).end(); ++it) {
            vector<int> temp1;
            stringstream test2(*it);

            while (getline(test2, segment, '*')) {
                temp1.push_back(stoi(segment));
            }

            temp2.push_back(temp1);
        }

        key.push_back(temp2);
    }

    return key;
}

/*
 * Gibt einen beliebigen privateKey auf die Konsole aus
 * */
void printPublicKey(vector<vector<vector<int>>> pKey) {

    for (int i = 0; i < pKey.size(); i++) {
        vector<vector<int>> tmp = pKey.at(i);
        printf("Formel %d:\n", i);
        for (int j = 0; j < tmp.size(); j++) {
            vector<int> tmp2 = tmp.at(j);

            printf("\t Teil Multiplication %d:\n", j);

            for (vector<int>::iterator it = tmp2.begin(); it != tmp2.end(); ++it) {
                printf("\t\t%d\n", *it);
            }
        }
    }

    printf("\n");

    return;
}

/*
 * Verschlüsselt einen Klartext mit einem belibigen publicKey
 * */
vector<int> encrypt(vector<int> clear, vector<vector<vector<int>>> pKey) {
    vector<int> chi;
    int cipher, offset = 0;

    for (int o = 0; o < clear.size(); o += pKey.size()) {

        if (o != 0) {
            offset = o;
        }

        // Geht jede Funktion nacheinander durch
        for (int i = 0; i < pKey.size(); i++) {
            vector<vector<int>> funktion = pKey.at(i);
            cipher = 0;

            /*
             * Geht alle Multiplicationen durch und addiert die einelnen Ergebnisse
             *
             * Beispiel x_2 * x_4
             * */
            for (int j = 0; j < funktion.size(); j++) {
                vector<int> teilFunktion = funktion.at(j);

                /*
                 * Da nicht jeder Teil der Funktion aus zwei Operanden besteht muss daruf geprüft werden
                 *
                 * Beispiel x_2 * x_4 oder x_3
                 * */
                if (teilFunktion.size() == 1) {
                    cipher += clear.at(offset + teilFunktion.at(0) - 1);
                } else if (teilFunktion.size() == 2) {
                    cipher += clear.at(offset + teilFunktion.at(0) - 1) * clear.at(offset + teilFunktion.at(1) - 1);
                }

            }

            /*
             * Das Ergebniss der gesamten Funktion wird Modulo 2 gerechnet und in den Vector als chiText gespeichert
             * */
            cipher = cipher % 2;
            chi.push_back(cipher);
        }

    }
    return chi;
}

/*
 * Erzeugt aus einem String eine Vector mit dem Klartext
 * */
vector<int> createClearVector(string clear, char c) {
    vector<int> out;
    stringstream test(clear);
    string segment;

    while (getline(test, segment, c)) {
        out.push_back(stoi(segment));
    }

    return out;
}

/*
 *
 * */
NTL::mat_GF2 createTriangleMatrix(vector<int> clear, vector<int> chi, int l) {
    NTL::mat_GF2 triangle;
    triangle.SetDims((clear.size() / l), l * l);
    NTL::clear(triangle);
    int m, n = 0;

    NTL::mat_GF2 vec;
    NTL::mat_GF2 mat;
    NTL::mat_GF2 erg;

    vec.SetDims(l, 1);
    mat.SetDims(1, l);

    for (int i = 0; i < clear.size(); i += l) {
        NTL::clear(vec);
        NTL::clear(mat);
        NTL::clear(erg);

        for (int j = i; j < (i + l); j++) {
            vec[j - i][0] = clear.at(j);
            mat[0][j - i] = chi.at(j);
        }
        NTL::mul(erg, vec, mat);

        m = 0;
        for (int a = 0; a < erg.NumRows(); a++) {
            for (int b = 0; b < erg.NumCols(); b++) {
                triangle[n][m] = erg.get(a, b);
                m++;
            }
        }
        n++;
    }
    return triangle;
}

/*
 * Eurzeugt die spezielle Lösung für die Matrix
 * */
vector<NTL::mat_GF2> matrixAufrollen(NTL::mat_GF2 dummy) {
    vector<NTL::mat_GF2> spezLoes;
    NTL::mat_GF2 triMat = dummy;
    int rowMax = -1;

    int test = 0;
    for (int i = 0; i < triMat.NumRows(); i++) {

        for (int j = 0; j < triMat.NumCols(); j++) {
            if (NTL::IsZero(triMat[i][j])) {
                test++;
            }
            if (test == triMat.NumCols()) {
                rowMax = i - 1;
                break;
            }
        }

        if (rowMax != -1) {
            break;
        }
        test = 0;
    }

    if (rowMax == -1) {
        rowMax = triMat.NumRows()-1;
    }

    if (!(NTL::IsZero(triMat))) {
        int col = triMat.NumCols();

        printf("\n");

        vector<int> expectedOne;
        vector<int> freeVar;
        vector<int> rowJump;
        expectedOne.push_back(rowMax - 1);
        expectedOne.push_back(col - 1);

        for (int i = rowMax; i >= 0; i--) {
            if (!(NTL::IsZero(triMat[i][expectedOne.at(1)]))) {
                int check = 0;
                int indexOfNull = -1;

                for (int o = expectedOne.at(1) - 1; o >= 0; o--) {
                    if (!(NTL::IsZero(triMat[i][o]))) {
                        check++;
                        indexOfNull = o;
                    }
                }

                if (check != 0) {

                    for (int x = expectedOne.at(1)-1; x > (indexOfNull - 1); x--) {
                        freeVar.push_back(expectedOne.at(1));
                        rowJump.push_back(i);
                        expectedOne.at(1)--;
                    }

                    expectedOne.at(1) = indexOfNull - 1;
                } else {
                    expectedOne.at(1)--;
                }

            } else {
                int indexOfNull = -1;

                for (int o = expectedOne.at(1); o >= 0; o--) {
                    if (!(NTL::IsZero(triMat[i][o]))) {
                        indexOfNull = o;
                    }
                }

                /*
                freeVar.push_back(expectedOne.at(1));
                rowJump.push_back(i);
                 */

                for (int x = expectedOne.at(1)-1; x > (indexOfNull - 1); x--) {
                    freeVar.push_back(expectedOne.at(1));
                    rowJump.push_back(i);
                    expectedOne.at(1)--;
                }

                expectedOne.at(1) = indexOfNull - 1;

            }

        }

        cout << "Liste aller freien Variablen: " << endl;
        for (int v = 0; v < freeVar.size(); v++) {
            cout << "x_" << freeVar[v]+1 << endl;
        }
        cout << endl;

        for (int v = 0; v < freeVar.size(); v++) {
            NTL::mat_GF2 vec;
            vec.SetDims(triMat.NumCols(), 1);

            vec[freeVar[v]][0] = 1;

            //cout << "Es wurde x_" << freeVar[v] + 1  << " auf Eins gesetzt\n" << vec << "\n" << endl;

            int index = triMat.NumCols() - 1;
            for (int w = rowMax; w >= 0; w--) {
                int test = 0;

                for (int g = 0; g < rowJump.size(); g++) {
                    if (w == rowJump[g]) {
                        index--;
                    }
                }

                for (int g = triMat.NumCols() - 1; g >= 0; g--) {
                    if (!(NTL::IsZero(triMat[w][g]))) {
                        if (!(NTL::IsZero(vec[g][0]))) {
                            test += 1;
                        }
                    }
                }

                if ((test % 2) == 1) {
                    vec[index][0] = 1;
                }
                index--;
            }

            spezLoes.push_back(vec);
        }
    }

    cout << "Liste aller Speziellen Lösungen: " << endl;
    for (int v = 0; v < spezLoes.size(); v++) {
        cout << spezLoes[v] << "\n" << endl;
    }
    cout << endl;

    return spezLoes;
}

NTL::mat_GF2 createBasis(vector<NTL::mat_GF2> specialSolution, int breite, vector<int> chi) {
    vector<NTL::mat_GF2> matrizen;

    int offset = 0;
    for (int i = 0; i < breite; i++) {

        NTL::mat_GF2 mat;
        mat.SetDims(specialSolution.size(), breite);

        for (int n = 0; n < specialSolution.size(); n++) {
            for (int o = 0; o < breite; o++) {
                mat[n][o] = (specialSolution.at(n))[o + offset][0];
            }
        }

        matrizen.push_back(mat);
        offset += breite;
        //cout << mat << endl;
    }

    vector<vector<vector<int>>> xyFormel;

    for (int i = 0; i < specialSolution.size(); i++) {
        vector<vector<int>> tmp2;
        for (int n = 0; n < matrizen.size(); n++) {
            for (int o = 0; o < breite; o++) {
                if (!(NTL::IsZero((matrizen.at(n))[i][o]))) {
                    vector<int> tmp;
                    tmp.push_back(n+1);
                    tmp.push_back(o);
                    tmp2.push_back(tmp);
                }
            }
        }
        xyFormel.push_back(tmp2);
    }

    vector<vector<int>> lgsNachChi;

    for (int i = 0; i < xyFormel.size(); i++) {
        vector<int> tmp;

        for (int n = 0; n < xyFormel.at(i).size(); n++) {
            for (int o = 0; o < xyFormel.at(i).at(n).size()-1; o++) {
                int t = 0;
                t = xyFormel.at(i).at(n).at(0) * chi.at(xyFormel.at(i).at(n).at(1));
                if (t > 0) {
                    tmp.push_back(xyFormel.at(i).at(n).at(0));
                }
            }
        }
        lgsNachChi.push_back(tmp);

    }


    vector<vector<int>> result;

    for (int i = 0; i < lgsNachChi.size(); i++) {
        vector<int> tmp;
        sort(lgsNachChi.at(i).begin(), lgsNachChi.at(i).end());

        for(auto it = std::cbegin(lgsNachChi.at(i)); it != std::cend(lgsNachChi.at(i)); ) {

            int dups = std::count(it, std::cend(lgsNachChi.at(i)), *it);
            if ( (dups % 2) == 1 )
                tmp.push_back(*it-1);
            for(auto last = *it;*++it == last;);
        }
        result.push_back(tmp);
    }

    NTL::mat_GF2 ende;
    ende.SetDims(lgsNachChi.size(), breite);

    for (int i = 0; i < result.size(); i++) {
        for (int n = 0; n < result.at(i).size(); n++) {
            ende[i][result.at(i).at(n)] = 1;
        }
    }

    //cout << ende << "\n" << endl;

    NTL::gauss(ende);

    cout << ende << "\n" << endl;

    //printf("Test");

    return ende;

}

void angriff(char pub[], string clear, char seperator, vector<int> doof) {

    /*
     * Erzeugt aus dem String den für die Berechnung benötigten Vector
     * */
    vector<vector<vector<int>>> publicK;
    publicK = createPublicKey(pub);

    vector<int> klartext;
    klartext = createClearVector(clear, seperator);

    // Erzeugen des Chitextes
    vector<int> chiText;
    chiText = encrypt(klartext, publicK);

    NTL::mat_GF2 a;
    a = createTriangleMatrix(klartext, chiText, publicK.size());

    cout << a << "\n" << endl;

    NTL::gauss(a);

    cout << a << "\n" << endl;

    vector<NTL::mat_GF2> specialSolution;
    specialSolution = matrixAufrollen(a);


    NTL::mat_GF2 b;
    b = createBasis(specialSolution, publicK.size(), doof);

    vector<NTL::mat_GF2> c;
    c = matrixAufrollen(b);

    cout << c.at(0) << "\n" << endl;

}

int main() {

    // Einlesen und aufbereiten des PublicKey

    char publicKey[] = "x_1*x_3 + x_2*x_3 + x_2,\n"
                       "    x_1*x_3 + x_1 + x_2 + x_3,\n"
                       "    x_1*x_2 + x_3";

    /*
    char publicKey[] = "x_1*x_3 + x_2*x_3 + x_1*x_5 + x_2*x_5 + x_3*x_5 + x_4*x_5 + x_1,\n"
                       "    x_1*x_2 + x_1*x_3 + x_2*x_3 + x_3*x_4 + x_1*x_5 + x_4*x_5 + x_2 + x_3,\n"
                       "    x_1*x_2 + x_1*x_3 + x_1*x_4 + x_2*x_4 + x_3*x_4 + x_1*x_5 + x_2*x_5 + x_1 + \n"
                       "        x_2 + x_3,\n"
                       "    x_1*x_3 + x_2*x_3 + x_2*x_4 + x_4*x_5 + x_2 + x_4,\n"
                       "    x_1*x_2 + x_1*x_4 + x_3*x_4 + x_4*x_5 + x_1 + x_2 + x_5";
    */
    // Einlesen und aufbereiten des Klartextes
    string clear = "0 1 1 0 0 0 1 0 1 1 0 0 0 0 1 0 1 0 1 1 0 1 1 1 1 0 1 0 1 1 1 0 0 0 1 0 1 0 1 0 1 1 1 1 1 0 0 0 0 0 1 0 1 0";
    //string clear = "0 1 1 1 0 1 0 1 0 0 0 0 1 1 1 1 1 0 1 0 0 1 1 1 1 0 0 1 1 1 1 0 1 1 0 1 1 1 1 0 1 1 1 1 0 1 1 1 0 0 1 1 1 1 1 1 0 1 0 1 0 1 1 1 0 1 0 0 0 1 0 1 1 0 0 0 1 0 1 1 1 0 1 1 1 0 0 0 0 0 0 0 0 1 1 1 0 0 0 0 1 0 0 1 1 0 0 0 0 0 0 1 1 0 1 1 0 1 0 0 1 0 1 1 1 0 1 1 1 0 1 0 1 1 1 0 0 0 0 0 1 1 0 1 0 0 0 1 1 0 1 0 0 1 0 0 0 0 0 0 1 0 0 0 1 0 1 0 1 0 0 1 1 0 0 0 1 0 0 0 1 0 1 0 0 1 1 0 0 0 1 1 1 1 0 0 0 1 1 1 0 1 1 1 0 1 1 1 0 1 1 1 0 0 1 1 1 1 1 0 1 1 1 0 1 1 0 1 0 1 0 1 1 1 0 1 0 0 1 0 1 0 0 0 0 1 0 1 1 1";

    // zu entschlüsselnder Text
    vector<int> chi = {1,1,1};
    //vector<int> chi = {1,0,0,0,1};

    angriff(publicKey, clear, ' ', chi);


    return 0;
}
