#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <NTL/mat_GF2.h>

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
    triangle.SetDims((clear.size()/l), l*l);
    NTL::clear(triangle);
    int m, n = 0;

    NTL::mat_GF2 vec;
    NTL::mat_GF2 mat;
    NTL::mat_GF2 erg;

    vec.SetDims(l,1);
    mat.SetDims(1, l);

    for (int i = 0; i < clear.size(); i += l) {
        NTL::clear(vec);
        NTL::clear(mat);
        NTL::clear(erg);

        for (int j = i; j < (i+l); j++) {
            vec[j-i][0] = clear.at(j);
            mat[0][j-i] = chi.at(j);
        }
        NTL::mul(erg, vec, mat);

        m = 0;
        for (int a = 0 ; a < erg.NumRows(); a++) {
            for (int b = 0 ; b < erg.NumCols(); b++) {
                triangle[n][m] = erg.get(a,b);
                m++;
            }
        }
        n++;
    }
    return triangle;
}

/*
 * Erzeugt aus der Matrix eine Ausgabeform die für WolframAlpha geeignet ist
 * */
void wolframAlphaExport(NTL::mat_GF2 m) {
    printf("{");
    for (int a = 0 ; a < m.NumRows(); a++) {
        printf("{");
        for (int b = 0 ; b < m.NumCols(); b++) {
            if (b < (m.NumCols()-1)) {
                printf("%d,", m.get(a,b));
            } else {
                printf("%d", m.get(a,b));
            }
        }
        if (a < (m.NumRows()-1)) {
            printf("},");
        } else {
            printf("}");
        }
    }
    printf("}\n");
}

/*
 * Eurzeugt die spezielle Lösung für die Matrix
 * */
NTL::mat_GF2 matrixAufrollen(NTL::mat_GF2 triMat) {
    NTL::mat_GF2 spezLoes;
    int rowMax = -1;

    int test = 0;
    for (int i = 0; i < triMat.NumRows(); i++) {

        for (int j = 0; j < triMat.NumCols(); j++) {
            if (NTL::IsZero(triMat[i][j])) {
                test++;
            }
            printf("%d ", test);
            if (test == triMat.NumCols()) {
                rowMax = i-1;
                break;
            }
        }

        if (rowMax != -1) {
            break;
        }
        printf("\n");
        test = 0;
    }

    if (!(NTL::IsZero(triMat))) {
        int col = triMat.NumCols();

        vector<vector<int>> freeVar;

        for (int i = 0; i < rowMax; i++) {
            for (int j = 0; j < col; j++) {
                if (!(NTL::IsZero(triMat[i][j]))) {
                    freeVar.push_back(vector<int> (i, j));
                }
            }
        }

        for (int i = rowMax; i == 0; i--) {
            for (int j = col; j == 0; j--) {

            }
        }

    }

    return spezLoes;
}

void angriff(char pub[], string clear, char seperator) {

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

    matrixAufrollen(a);

}

int main() {

    // Einlesen und aufbereiten des PublicKey
    char publicKey[] = "x_1*x_3 + x_2*x_3 + x_2,\n"
                       "    x_1*x_3 + x_1 + x_2 + x_3,\n"
                       "    x_1*x_2 + x_3";

    vector<vector<vector<int>>> publicK;
    publicK = createPublicKey(publicKey);

    // printPublicKey(publicK);

    // Einlesen und aufbereiten des Klartextes
    string clear = "0 1 1 0 0 0 1 0 1 1 0 0 0 0 1 0 1 0 1 1 0 1 1 1 1 0 1 0 1 1 1 0 0 0 1 0 1 0 1 0 1 1 1 1 1 0 0 0 0 0 1 0 1 0";

    angriff(publicKey, clear, ' ');

    /*
    NTL::mat_GF2 test1;
    test1.SetDims(7,9);

    test1[0][0] = 1;
    test1[1][1] = 1;
    test1[2][2] = 1;
    test1[3][3] = 1;
    test1[4][4] = 1;
    test1[5][5] = 1;
    test1[6][7] = 1;

    test1[0][8] = 1;

    test1[2][6] = 1;
    test1[2][8] = 1;

    test1[3][6] = 1;
    test1[3][8] = 1;

    test1[4][6] = 1;
    test1[4][8] = 1;

    test1[5][8] = 1;

    test1[6][8] = 1;

    cout << test1 << "\n" << endl;

    NTL::mat_GF2 test21;
    test21.SetDims(9,1);

    test21[0][0] = 1;
    test21[1][0] = 0;
    test21[2][0] = 0;
    test21[3][0] = 0;
    test21[4][0] = 0;
    test21[5][0] = 1;
    test21[6][0] = 1;
    test21[7][0] = 1;
    test21[8][0] = 1;

    cout << test21 << "\n" << endl;

    NTL::mat_GF2 test22;
    test22.SetDims(9,1);

    test22[0][0] = 1;
    test22[1][0] = 0;
    test22[2][0] = 0;
    test22[3][0] = 0;
    test22[4][0] = 0;
    test22[5][0] = 1;
    test22[6][0] = 1;
    test22[7][0] = 1;
    test22[8][0] = 1;

    cout << test22 << "\n" << endl;

    NTL::mat_GF2 test23;

    NTL::add(test23, test21, test22);

    NTL::mat_GF2 test3;

    NTL::mul(test3, test1, test23);

    cout << test3 << "\n" << endl;
    cout << NTL::IsZero(test3) << "\n" << endl;
    */

    return 0;
}
