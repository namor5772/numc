// numc.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Release x64. It works on Win10 machines...

#include <iostream>


// global variable declarations
char fs[5000][40]; 
int fsn[5000];
int fsi1[5000];
int fsii[5000];
int fsp = -1;

int av[6];
int ag;
int n, k, m, q, cn, ci;
int a[6];
int b[5];

// core bracketing display and calculation variables and arrays
int TMP = 6;
char br[7][40]; // stack for expression strings
int si[7]; // parallel integer stack for expression values
int so[7]; // stack for component operator code (-1 if primitive component)
int ssp[7]; // pointer for internal stacks
char sbr[7][7][40]; // internal stack for expression strings
int ssi[7][7]; // internal stack for expression values 
int sso[7][7]; // internal stack for component operators. sso[][i] is operator between sbr[][i-1] and sbr[][i] 
int ssa[7][7]; // internal stack for 1st value for component ordering
int ssb[7][7]; // internal stack for 2nd value for component ordering
int ssx[7][7]; // internal stack for internal stack ordering

int ERINT = -6666;
char c[42][12] = {
    "aaaaaa43210","aaaaa3a4210","aaaa2aa4310","aaaaa32a410","aaaa2a3a410","aaa1aaa4320","aaa1aa3a420",
    "aaaa21aa430","aaa1a2aa430","aaaaa321a40","aaaa2a31a40","aaa1aa32a40","aaaa21a3a40","aaa1a2a3a40",
    "aaaaa3210a4","aaaa2a310a4","aaa1aa320a4","aaaa21a30a4","aaa1a2a30a4","aa0aaa321a4","aa0aa2a31a4",
    "aaa10aa32a4","aa0a1aa32a4","aaaa210a3a4","aaa1a20a3a4","aa0aa31a3a4","aaa10a2a3a4","aa0a1a2a3a4",
    "aa0aaaa4321","aa0aaa3a421","aa0aa2aa431","aa0aaa32a41","aa0aa2a3a41","aaa10aaa432","aa0a2aaa432",
    "aaa10aa3a42","aa0a2aa3a42","aaaa210aa43","aaa2a20aa43","aa0aa21aa43","aaa10a2aa43","aa0a1a2aa43"
};


// Calculates Hash integer for char array sbr[i][j][] 
int strHash(int i, int j) {
    int t = 0;
    int iHash = 0;

    // adds together the ascii codes for all the characters in a char array,
    // ignoring end char '\0'. Duplicate hash values should not be a problem.
    while (sbr[i][j][t] != '\0') {
        iHash += int(sbr[i][j][t]);
        t++;
    }
    return iHash;
}


// Maps operator codes to chars
char OpMap(int o) {
    if (o == 0) { return '+'; }
    else if (o == 1) { return '-'; }
    else if (o == 2) { return '*'; }
    else if (o == 3) { return '/'; }
    else { return '?'; }
}


// THE operator function
int Op(int o, int x, int y) {
    // o is an integer indicating basic operators.
    // in particular : 0 = +, 1 = -, 2 = *, 3 = /
    // All operations are assumed to generate integers

    if ((x == ERINT) || (y == ERINT)) {
        return ERINT;
    }
    else if (o == 0) { // + operation
        return x + y;
    }
    else if (o == 1) { // - operation
        if (x < y) {
            return ERINT;
        }
        else {
            return x - y;
        }
    }
    else if (o == 2) { // * operation
        return x * y;
    }
    else { // try integer division
        if (y == 0) { // cannot divide by 0
            return ERINT;
        }
        else if ((x / y) * y != x) { // x / y must be an integer
            return ERINT;
        }
        else { // integer devision possible
            return x / y;
        }
    }
}

int TryPermCombBracTriplet(char cb[12]) {
    // looping through all characters in the bracketing string (cb)
    int o, y, x;
    int i, j = 0;
    char r;
//    int si[6]; // integer "stack"
    int pi = -1; // pointer to end of stack

    for (i = 0; i < 11 - 2 * ci; i++) {
        r = cb[i];
        if (r == 'a') {
            // push number as specified by permutation onto stack
            pi += 1;
            si[pi] = av[a[j]];
            j += 1;
        }
        else {
            // perform operation
            o = b[(r - '0') - ci];
            y = si[pi];
            x = si[pi - 1];
            pi -= 1;
            si[pi] = Op(o, x, y);
        }
    }
    return si[pi];
}

void CreateSortedExpressionString_Enhanced(char cb[12]) {
    int i, t, u, v, swp;
    int o; // operation code
    char r; // a bracketing string character
    int xo, yo, x, y; // previous two operation codes and values
    int j; // pointer to input number
    int pi; // pointer to expression calculation stack

    j = -1; // pointer to input number
    pi = -1; // pointer to end of expression calculation stack

    // looping through all characters in the bracketing string (cb)
    for (i = 0; i < 11 - 2 * ci; i++) {
        r = cb[i];
        if (r == 'a') {
            j += 1; // increment pointer to input number
            pi += 1; // increment expression calculation stack pointer

            // push string representation of number and the number itself onto expression calculation stacks
            v = av[a[j]];
            sprintf_s(br[pi], "%d", v);
//            sprintf(br[pi], "%d", v);
            si[pi] = v; // push onto si[]
            so[pi] = -1; // push onto so[]
            ssp[pi] = -1; // push onto ssp[]
//            printf("\n");
//            printf("%s\t", br[pi]);
        }
        else {
            o = b[(r - '0') - ci]; // determine operation code from reverse Polish bracketing array

            // determine base operations for last two expressions in stack
            xo = so[pi - 1];
            yo = so[pi];

            // determine values for last two expressions in stack
            x = si[pi - 1];
            y = si[pi];

            if ( ((o == 0) || (o == 1)) && ((xo == 0) || (yo == 0) || (xo == 1) || (yo == 1)) ) {
                // consolidation of + & - expressions is possible

                // create the expression value
                si[TMP] = Op(o, x, y);

                // save the operation code
                so[TMP] = o;

                // update local stack pointer
                if ((xo == 0)||(xo == 1)) { u = ssp[pi - 1] + 1; }
                else { u = 1; }
                if ((yo == 0)||(yo == 1)) { u = u + ssp[pi] + 1; }
                else { u = u + 1; }
                ssp[TMP] = u - 1;

                // building up sbr[], ssi[], sso[], ssa[] and ssb[] for different cases
                if ( ((xo != 0) && (xo != 1)) && ((yo == 0) || (yo == 1)) ) {
                    u = 0;
                    t = 0; while (br[pi - 1][t] != '\0') { sbr[TMP][u][t] = br[pi - 1][t]; t++; }; sbr[TMP][u][t] = '\0';
                    ssi[TMP][u] = x;
                    sso[TMP][u] = 0; // assume addition
                    ssa[TMP][u] = ssi[TMP][u];
                    ssb[TMP][u] = strHash(TMP, u);
                    u = 1;

                    if (o == 0) { // adding y to bottom of internal stack so no adjustments necessary 
                        for (v = 0; v <= ssp[pi]; v++) {
                            t = 0; while (sbr[pi][v][t] != '\0') { sbr[TMP][u + v][t] = sbr[pi][v][t]; t++; }; sbr[TMP][u + v][t] = '\0';
                            ssi[TMP][u + v] = ssi[pi][v];
                            sso[TMP][u + v] = sso[pi][v]; 
                            ssa[TMP][u + v] = ssa[pi][v]; 
                            ssb[TMP][u + v] = ssb[pi][v];
                        }
                    }
                    else { // if (o == 1) 'subtracting y from bottom of internal stack, must flip signs as necessary
                        for (v = 0; v <= ssp[pi]; v++) {
                            t = 0; while (sbr[pi][v][t] != '\0') { sbr[TMP][u + v][t] = sbr[pi][v][t]; t++; }; sbr[TMP][u + v][t] = '\0';
                            ssi[TMP][u + v] = ssi[pi][v];
                            if (v == 0) {
                                sso[TMP][u + v] = o; // first element has to be subtracted (note: o==1)
                                ssa[TMP][u + v] = ssa[pi][v] * -1; // make negative for sorting
                            }
                            else {
                                sso[TMP][u + v] = ((sso[pi][v] == 0) ? 1 : 0); // operators/sign flipped 
                                ssa[TMP][u + v] = (ssa[pi][v]) * -1; // flip sign
                            }
                            ssb[TMP][u + v] = ssb[pi][v];
                        }
                    }
                }
                else if ( ((xo == 0) || (xo == 1)) && ((yo != 0) && (yo != 1)) ) {
                    for (u = 0; u <= ssp[pi - 1]; u++) {
                        t = 0; while (sbr[pi - 1][u][t] != '\0') { sbr[TMP][u][t] = sbr[pi - 1][u][t]; t++; }; sbr[TMP][u][t] = '\0';
                        ssi[TMP][u] = ssi[pi - 1][u];
                        sso[TMP][u] = ((u==0) ? 0 : sso[pi-1][u]); // assume addition for first element and leave others unchanged
                        ssa[TMP][u] = ssa[pi-1][u];
                        ssb[TMP][u] = ssb[pi-1][u];
                    }
                    u = ssp[pi - 1] + 1;

                    t = 0; while (br[pi][t] != '\0') { sbr[TMP][u][t] = br[pi][t]; t++; }; sbr[TMP][u][t] = '\0';
                    ssi[TMP][u] = y;
                    sso[TMP][u] = o;
                    ssa[TMP][u] = ((o == 0) ? 1 : -1) * y;
                    ssb[TMP][u] = strHash(TMP, u);
                }
                else { // if (xo==0 || xo==1) && (yo==0 || yo==1)
                    for (u = 0; u <= ssp[pi - 1]; u++) {
                        t = 0; while (sbr[pi - 1][u][t] != '\0') { sbr[TMP][u][t] = sbr[pi - 1][u][t]; t++; }; sbr[TMP][u][t] = '\0';
                        ssi[TMP][u] = ssi[pi - 1][u];
                        sso[TMP][u] = ((u == 0) ? 0 : sso[pi - 1][u]); // assume addition for first element and leave others unchanged
                        ssa[TMP][u] = ssa[pi - 1][u];
                        ssb[TMP][u] = ssb[pi - 1][u];
                    }
                    u = ssp[pi - 1] + 1;

                    if (o == 0) { // adding y to bottom of internal stack so no adjustments necessary 
                        for (v = 0; v <= ssp[pi]; v++) {
                            t = 0; while (sbr[pi][v][t] != '\0') { sbr[TMP][u + v][t] = sbr[pi][v][t]; t++; }; sbr[TMP][u + v][t] = '\0';
                            ssi[TMP][u + v] = ssi[pi][v];
                            sso[TMP][u + v] = ((v == 0) ? 0 : sso[pi][v]);
                            ssa[TMP][u + v] = ssa[pi][v];
                            ssb[TMP][u + v] = ssb[pi][v];
                        }
                    }
                    else { // if (o == 1) 'subtracting' y from bottom of internal stack, must flip signs as necessary
                        for (v = 0; v <= ssp[pi]; v++) {
                            t = 0; while (sbr[pi][v][t] != '\0') { sbr[TMP][u + v][t] = sbr[pi][v][t]; t++; }; sbr[TMP][u + v][t] = '\0';
                            ssi[TMP][u + v] = ssi[pi][v];
                            if (v == 0) {
                                sso[TMP][u + v] = o; // first element has to be subtracted (note: o==1)
                                ssa[TMP][u + v] = ssa[pi][v] * -1; // make negative for sorting
                            }
                            else {
                                sso[TMP][u + v] = ((sso[pi][v] == 0) ? 1 : 0); // operators/sign flipped 
                                ssa[TMP][u + v] = (ssa[pi][v]) * -1; // flip sign
                            }
                            ssb[TMP][u + v] = ssb[pi][v];
                        }
                    }
                }

                // determine expression sorting order with simple bubble sort
                for (u = 0; u <= ssp[TMP]; u++) { ssx[TMP][u] = u; }
                for (u = 0; u < ssp[TMP]; u++) {
                    for (v = 0; v < (ssp[TMP] - u); v++) {
                        if ((ssa[TMP][ssx[TMP][v]]) < (ssa[TMP][ssx[TMP][v+1]])) {
                            swp = ssx[TMP][v];
                            ssx[TMP][v] = ssx[TMP][v+1];
                            ssx[TMP][v + 1] = swp;
                        }
                    }
                }
/*
                printf("\n");
                for (v = 0; v <= ssp[TMP]; v++) { printf("%i\t", ssx[TMP][v]); }
                printf("\n");
                for (v = 0; v <= ssp[TMP]; v++) { printf("%i\t", ssa[TMP][v]); }
                printf("\n");
                for (v = 0; v <= ssp[TMP]; v++) { printf("%i\t", ssb[TMP][v]); }
                printf("\n");
                for (v = 0; v <= ssp[TMP]; v++) { printf("%i\t", sso[TMP][v]); }
*/
                // create the expression string
                u = 0;
                br[TMP][u] = '(';
                u += 1;
                t = 0; while (sbr[TMP][ssx[TMP][0]][t] != '\0') { br[TMP][t + u] = sbr[TMP][ssx[TMP][0]][t]; t++; };
                u += t;
                for (v = 1; v <= ssp[TMP]; v++) {
                    br[TMP][u] = OpMap(sso[TMP][ssx[TMP][v]]);
                    u += 1;
                    t = 0; while (sbr[TMP][ssx[TMP][v]][t] != '\0') { br[TMP][t + u] = sbr[TMP][ssx[TMP][v]][t]; t++; };
                    u += t;
                }
                br[TMP][u] = ')';
                u += 1;
                br[TMP][u] = '\0';

            }
            else if (((o == 2) || (o == 3)) && ((xo == 2) || (yo == 2) || (xo == 3) || (yo == 3))) {
                // consolidation of * & / expressions is possible

                // create the expression value
                si[TMP] = Op(o, x, y);

                // save the operation code
                so[TMP] = o;

                // update local stack pointer
                if ((xo == 2) || (xo == 3)) { u = ssp[pi - 1] + 1; }
                else { u = 1; }
                if ((yo == 2) || (yo == 3)) { u = u + ssp[pi] + 1; }
                else { u = u + 1; }
                ssp[TMP] = u - 1;

                // building up sbr[], ssi[], sso[], ssa[] and ssb[] for different cases
                if (((xo != 2) && (xo != 3)) && ((yo == 2) || (yo == 3))) {
                    u = 0;
                    t = 0; while (br[pi - 1][t] != '\0') { sbr[TMP][u][t] = br[pi - 1][t]; t++; }; sbr[TMP][u][t] = '\0';
                    ssi[TMP][u] = x;
                    sso[TMP][u] = 2; // assume multiplication
                    ssa[TMP][u] = ssi[TMP][u];
                    ssb[TMP][u] = strHash(TMP, u);
                    u = 1;

                    if (o == 2) { // adding y to bottom of internal stack so no adjustments necessary 
                        for (v = 0; v <= ssp[pi]; v++) {
                            t = 0; while (sbr[pi][v][t] != '\0') { sbr[TMP][u + v][t] = sbr[pi][v][t]; t++; }; sbr[TMP][u + v][t] = '\0';
                            ssi[TMP][u + v] = ssi[pi][v];
                            sso[TMP][u + v] = sso[pi][v];
                            ssa[TMP][u + v] = ssa[pi][v];
                            ssb[TMP][u + v] = ssb[pi][v];
                        }
                    }
                    else { // if (o == 3) 'dividing' y from bottom of internal stack, must flip signs as necessary
                        for (v = 0; v <= ssp[pi]; v++) {
                            t = 0; while (sbr[pi][v][t] != '\0') { sbr[TMP][u + v][t] = sbr[pi][v][t]; t++; }; sbr[TMP][u + v][t] = '\0';
                            ssi[TMP][u + v] = ssi[pi][v];
                            if (v == 0) {
                                sso[TMP][u + v] = o; // first element has to be divided (note: o==3)
                                ssa[TMP][u + v] = ssa[pi][v] * -1; // make negative for sorting
                            }
                            else {
                                sso[TMP][u + v] = ((sso[pi][v] == 2) ? 3 : 2); // operators/sign flipped between / and * 
                                ssa[TMP][u + v] = (ssa[pi][v]) * -1; // flip sign
                            }
                            ssb[TMP][u + v] = ssb[pi][v];
                        }
                    }
                }
                else if (((xo == 2) || (xo == 3)) && ((yo != 2) && (yo != 3))) {
                    for (u = 0; u <= ssp[pi - 1]; u++) {
                        t = 0; while (sbr[pi - 1][u][t] != '\0') { sbr[TMP][u][t] = sbr[pi - 1][u][t]; t++; }; sbr[TMP][u][t] = '\0';
                        ssi[TMP][u] = ssi[pi - 1][u];
                        sso[TMP][u] = ((u == 0) ? 2 : sso[pi - 1][u]); // assume multiplication for first element and leave others unchanged
                        ssa[TMP][u] = ssa[pi - 1][u];
                        ssb[TMP][u] = ssb[pi - 1][u];
                    }
                    u = ssp[pi - 1] + 1;

                    t = 0; while (br[pi][t] != '\0') { sbr[TMP][u][t] = br[pi][t]; t++; }; sbr[TMP][u][t] = '\0';
                    ssi[TMP][u] = y;
                    sso[TMP][u] = o;
                    ssa[TMP][u] = ((o == 2) ? 1 : -1) * y;
                    ssb[TMP][u] = strHash(TMP, u);
                }
                else { // if (xo==2 || xo==3) && (yo==2 || yo==3)
                    for (u = 0; u <= ssp[pi - 1]; u++) {
                        t = 0; while (sbr[pi - 1][u][t] != '\0') { sbr[TMP][u][t] = sbr[pi - 1][u][t]; t++; }; sbr[TMP][u][t] = '\0';
                        ssi[TMP][u] = ssi[pi - 1][u];
                        sso[TMP][u] = ((u == 0) ? 2 : sso[pi - 1][u]); // assume multiplication for first element and leave others unchanged
                        ssa[TMP][u] = ssa[pi - 1][u];
                        ssb[TMP][u] = ssb[pi - 1][u];
                    }
                    u = ssp[pi - 1] + 1;

                    if (o == 2) { // multiplying y to bottom of internal stack so no adjustments necessary 
                        for (v = 0; v <= ssp[pi]; v++) {
                            t = 0; while (sbr[pi][v][t] != '\0') { sbr[TMP][u + v][t] = sbr[pi][v][t]; t++; }; sbr[TMP][u + v][t] = '\0';
                            ssi[TMP][u + v] = ssi[pi][v];
                            sso[TMP][u + v] = ((v == 0) ? 2 : sso[pi][v]);
                            ssa[TMP][u + v] = ssa[pi][v];
                            ssb[TMP][u + v] = ssb[pi][v];
                        }
                    }
                    else { // if (o == 3) 'dividing' y from bottom of internal stack, must flip signs as necessary
                        for (v = 0; v <= ssp[pi]; v++) {
                            t = 0; while (sbr[pi][v][t] != '\0') { sbr[TMP][u + v][t] = sbr[pi][v][t]; t++; }; sbr[TMP][u + v][t] = '\0';
                            ssi[TMP][u + v] = ssi[pi][v];
                            if (v == 0) {
                                sso[TMP][u + v] = o; // first element has to be divisor (note: o==3)
                                ssa[TMP][u + v] = ssa[pi][v] * -1; // make negative for sorting
                            }
                            else {
                                sso[TMP][u + v] = ((sso[pi][v] == 2) ? 3 : 2); // operators/sign flipped 
                                ssa[TMP][u + v] = (ssa[pi][v]) * -1; // flip sign
                            }
                            ssb[TMP][u + v] = ssb[pi][v];
                        }
                    }
                }

                // determine expression sorting order with simple bubble sort
                for (u = 0; u <= ssp[TMP]; u++) { ssx[TMP][u] = u; }
                for (u = 0; u < ssp[TMP]; u++) {
                    for (v = 0; v < (ssp[TMP] - u); v++) {
                        if ((ssa[TMP][ssx[TMP][v]]) < (ssa[TMP][ssx[TMP][v + 1]])) {
                            swp = ssx[TMP][v];
                            ssx[TMP][v] = ssx[TMP][v + 1];
                            ssx[TMP][v + 1] = swp;
                        }
                    }
                }
                /*
                                printf("\n");
                                for (v = 0; v <= ssp[TMP]; v++) { printf("%i\t", ssx[TMP][v]); }
                                printf("\n");
                                for (v = 0; v <= ssp[TMP]; v++) { printf("%i\t", ssa[TMP][v]); }
                                printf("\n");
                                for (v = 0; v <= ssp[TMP]; v++) { printf("%i\t", ssb[TMP][v]); }
                                printf("\n");
                                for (v = 0; v <= ssp[TMP]; v++) { printf("%i\t", sso[TMP][v]); }
                */
                // create the expression string
                u = 0;
                br[TMP][u] = '(';
                u += 1;
                t = 0; while (sbr[TMP][ssx[TMP][0]][t] != '\0') { br[TMP][t + u] = sbr[TMP][ssx[TMP][0]][t]; t++; };
                u += t;
                for (v = 1; v <= ssp[TMP]; v++) {
                    br[TMP][u] = OpMap(sso[TMP][ssx[TMP][v]]);
                    u += 1;
                    t = 0; while (sbr[TMP][ssx[TMP][v]][t] != '\0') { br[TMP][t + u] = sbr[TMP][ssx[TMP][v]][t]; t++; };
                    u += t;
                }
                br[TMP][u] = ')';
                u += 1;
                br[TMP][u] = '\0';
            }
            else {
                // no consolidation of previous two stack expressions is possible

                // create the expression value
                si[TMP] = Op(o, x, y);

                // save the operation code
                so[TMP] = o;

                // pop the last two strings from the stack into local TMP char and int arrays and int variables
                // also calculate auxillary stack values
                t = 0; while (br[pi][t] != '\0') { sbr[TMP][1][t] = br[pi][t]; t++; }; sbr[TMP][1][t] = '\0';
                ssi[TMP][1] = si[pi]; // y
                sso[TMP][1] = o;
                ssa[TMP][1] = (((sso[TMP][1] == 1) || (sso[TMP][1] == 3)) ? -1 : 1) * ssi[TMP][1];
                ssb[TMP][1] = strHash(TMP, 1);

                t = 0; while (br[pi - 1][t] != '\0') { sbr[TMP][0][t] = br[pi - 1][t]; t++; }; sbr[TMP][0][t] = '\0';
                ssi[TMP][0] = si[pi - 1]; // x
                sso[TMP][0] = ((o == 0) || (o == 1)) ? 0 : 2; // works ??
                ssa[TMP][0] = ssi[TMP][0];
                ssb[TMP][0] = strHash(TMP, 0);

                // calculate expression sorting order
                if (((ssa[TMP][0] < ssa[TMP][1]) || ((ssa[TMP][0] == ssa[TMP][1])) && (ssb[TMP][0] < ssb[TMP][1]))) {
                    ssx[TMP][0] = 1;
                    ssx[TMP][1] = 0;
                }
                else {
                    ssx[TMP][0] = 0;
                    ssx[TMP][1] = 1;
                }

                // create the expression string
                br[TMP][0] = '(';
                t = 0; while (sbr[TMP][ssx[TMP][0]][t] != '\0') { br[TMP][t + 1] = sbr[TMP][ssx[TMP][0]][t]; t++; };
                br[TMP][t + 1] = OpMap(o);
                u = 0; while (sbr[TMP][ssx[TMP][1]][u] != '\0') { br[TMP][t + u + 2] = sbr[TMP][ssx[TMP][1]][u]; u++; };
                br[TMP][t + u + 2] = ')';
                br[TMP][t + u + 3] = '\0';

                // update local stack pointer
                ssp[TMP] = 1; // to reflect the two component values
            }

            // decrement expression calculation stack pointer
            pi -= 1;

            // copy all stack data from the TMP to the pi element: br, si, so, ssp, sbr[], ssi[], sso[], ssa[], ssb[] and ssx[] 
            t = 0; while (br[TMP][t] != '\0') { br[pi][t] = br[TMP][t]; t++; } br[pi][t] = '\0'; // copy the char array br[TMP] into br[pi]
            si[pi] = si[TMP]; // copy si
            so[pi] = so[TMP]; // copy so
            ssp[pi] = ssp[TMP]; // copy ssp
            for (u = 0; u <= ssp[TMP]; u++) {
                t = 0; while (sbr[TMP][u][t] != '\0') { sbr[pi][u][t] = sbr[TMP][u][t]; t++; } sbr[pi][u][t] = '\0';// copy the char array sbr[TMP][u] into sbr[pi][u]
                ssi[pi][u] = ssi[TMP][u]; // copy ssi[][u]
                sso[pi][u] = sso[TMP][u]; // copy sso[][u]
                ssa[pi][u] = ssa[TMP][u]; // copy ssa[][u]
                ssb[pi][u] = ssb[TMP][u]; // copy ssb[][u]
                ssx[pi][u] = ssx[TMP][u]; // copy ssx[][u]
            }
 //           printf("\n");
 //           printf("%s\t", br[TMP]);
        }
    }
}

// Uses global variable a to generate the next permutation P(n, k)
bool GenerateNextPermutation() {
    bool NotFinishedP = true;
    int i, j, t, edge;
    int r[6];

    // find smallest j in (k...n - 1) where a[edge] < a[j]
    edge = k - 1;
    j = k;
    while ((j < n) && (a[edge] >= a[j])) j += 1;

    if (j < n) {
        // swap a[edge] and the just found a[j]
        t = a[edge]; a[edge] = a[j]; a[j] = t;
    }
    else {
        // reverse a[k] to a[n - 1]
        for (j = 0; j < n - k; j++) r[j] = a[k + j];
        for (j = 0; j < n - k; j++) a[k + j] = r[(n - 1) - (k + j)];

        // find rightmost ascent to left of edge
        i = edge - 1;
        while ((i >= 0) && (a[i] >= a[i + 1])) i -= 1;

        // if cannot find above, set NotFinishedP to False
        if (i < 0) {
            NotFinishedP = false;
        }
        else {
            // find largest j in (n - 1...i + 1) where a[i] < a[j]
            j = n - 1;
            while ((j > i) && (a[j] < a[i])) j -= 1;

            // swap a[i] and the just found a[j] (we can have i == j)
            t = a[i]; a[i] = a[j]; a[j] = t;

            // reverse a[i + 1] to a[n - 1]
            for (j = 0; j < n - i - 1; j++) r[j] = a[i + j + 1];
            for (j = 0; j < n - i - 1; j++) a[i + j + 1] = r[(n - 1) - (i + j + 1)];
        }
    }
    return NotFinishedP;
}

// Uses global variable b to generate the next combination C(m, q)
bool GenerateNextCombination() {
    int i = m - 1;
    bool IncrementC = true;

    //i = m - 1;
    IncrementC = true;
    while (IncrementC) {
        if (b[i] == q - 1) {
            b[i] = 0;
            i -= 1;
            if (i == -1) IncrementC = false;
        }
        else {
            b[i] += 1;
            IncrementC = false;
        }
    }
    return !(i == -1);
}

void CheckBlock1() {
    int i, i1 = 0;

    // search for ag in the input number array av[]
    for (i = 0; i < 6; i++) { if (av[i] == ag) { i1 = 1; break; } }

    printf("\n");

    if (i1 == 1) {
        printf("%i %i %i (%i) = %i\t", 1, i + 1, 1, av[i], ag);
        printf("\n");
        printf("1 SOLUTION FOUND USING 1 NUMBER, TESTED 6 CASES\t");
    }
    else if (i1 == 0) {
        printf("NO SOLUTIONS FOUND USING 1 NUMBER, TESTED 6 CASES\t");
    }
    printf("\n");
}

void CheckBlock(int xn, int xk, int xm, int xq, int xcn, int xci, int xt) {
    char cx[12];
    int i, j, res, i1, ii, t, u;
    bool NotFinishedP, NotFinishedC;
    bool isSame;
    int fsp = -1;

    // setup for particular size permutations, combinations and bracketing
    n = xn; k = xk; m = xm; q = xq; cn = xcn, ci = xci;
    i1 = 0; ii = 0;

    // initalize permutation array a[] in ascending order
    for (i = 0; i < n; i++) a[i] = i;

    NotFinishedP = true;
    while (NotFinishedP) {
        // do stuff with current permutation

        // start generating combinations
        for (i = 0; i < m; i++) b[i] = 0;

        NotFinishedC = true;
        while (NotFinishedC) {
            // do stuff with current permutation and combination pair

            // looping through the bracketing options
            for (i = 0; i < cn; i++) {
                // do stuff with current permutation, combination and bracketing triplet
                ii += 1;
                for (j = ci; j < 11 - ci; j++) cx[j - ci] = c[i][j];

                res = TryPermCombBracTriplet(cx);
                if (res == ag) {
                    CreateSortedExpressionString_Enhanced(cx);

                    // try to find br[0] in the fs[] array of strings
                    isSame = false;
                    u = 0;
                    while ((!isSame) && (u <= fsp)) {
                        isSame = true;

                        t = 0;
                        while ((br[0][t] != '\0') && (isSame)) {
                            isSame = (br[0][t] == fs[u][t]);
                            t += 1;
                        }
                        if (!isSame) u += 1; else fsn[u] += 1; 
                    }

                    if (!isSame) {
                        // apparently new solution, store it and print it
                        i1 += 1;
                        fsp += 1;
                        t = 0; while (br[0][t] != '\0') { fs[fsp][t] = br[0][t]; t++; }; fs[fsp][t] = '\0';
                        fsn[fsp] = 1;
                        fsi1[fsp] = i1;
                        fsii[fsp] = ii;

//                        printf("\n");
//                        printf("%i %i %s = %i\t", i1, ii, br[0], res);

                        //                       printf("\n");
                        //                       for (v = 0; v < xt; v++) { printf("%i\t", av[a[v]]); }; for (v = 0; v < xm; v++) { printf("%c\t", OpMap(b[v])); }; printf("%s\t", cx);
                    }
                }
            }
            // generate next combination
            NotFinishedC = GenerateNextCombination();
        }
        // generate next permutation
        NotFinishedP = GenerateNextPermutation();
    }

    // print out number of solution, cases tested, cases for solution, solution and result 
    printf("\n");
    for (i = 0; i <= fsp; i++) { printf("%i %i %i %s = %i\t", fsi1[i], fsii[i], fsn[i], fs[i], ag); printf("\n"); }

    if (i1 == 1) {
        printf("%i SOLUTION FOUND USING %i NUMBERS, TESTED %i CASES\t", i1, xt, ii);
    }
    else if (i1 == 0) {
        printf("NO SOLUTIONS FOUND USING %i NUMBERS, TESTED %i CASES\t", xt, ii);
    }
    else {
        printf("%i SOLUTIONS FOUND USING %i NUMBERS, TESTED %i CASES\t", i1, xt, ii);
    }
    printf("\n");
}

int main(int argc, char* argv[])
{
    int i;

    // extract the 6 trial numbers av[] and the result number ag
    for (i = 0; i < 6; i++) av[i] = atoi(argv[i + 1]);
    ag = atoi(argv[7]);

    // USING ANY 1, 2, 3, 4, 5 and 6 NUMBERS
    CheckBlock1(); // trivial case of one number match - included for complataeness
    CheckBlock(6, 2, 1, 4, 1, 4, 2);
    CheckBlock(6, 3, 2, 4, 2, 3, 3);
    CheckBlock(6, 4, 3, 4, 5, 2, 4);
    CheckBlock(6, 5, 4, 4, 14, 1, 5);
    CheckBlock(6, 5, 5, 4, 42, 0, 6);
}


