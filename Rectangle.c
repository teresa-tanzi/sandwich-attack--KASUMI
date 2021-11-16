/*-------------------------------------------------------------------------------------------
 *										Rectangle.c
 *-------------------------------------------------------------------------------------------
 *
 * An implementation of the related-key rectangle attack on the full KASUMI presented by 
 * Bihnam et al.
 *
 *-------------------------------------------------------------------------------------------*/

#include <stdio.h>      // printf()
#include <stdlib.h>		// rand(), srand()
#include <time.h>    	// time()
#include <math.h>       // pow()
#include "uthash.h"
#include "Kasumi.h"


static u8 *Ka;
static u8 Kb[16], Kc[16], Kd[16];

static void printHex(char name[], u8 text[], int n) {
    printf("%s:\t", name);
    for (int i = 0; i < n; i++)
        printf("%02x ", text[i]);
    printf("\n");
}

/*-------------------------------------------------------------------------------------------
 * start with a plaintexts quartet (P_a , P_b , P_c , P_d ) such that:
 * P_a xor P_b = P_c xor P_d = A = (0, 0, 0010_x, 0)
 * and use keys (K_a, K_b, K_c, K_d) such that
 * K_a xor K_b = K_c xor K_d = ∆K_ab =  (0, 0, 8000_x, 0, 0, 0, 0, 0)
 * K_a xor K_c = K_b xor K_d = ∆K_ac = (0, 0, 0, 0, 0, 0, 8000_x, 0)
 *-------------------------------------------------------------------------------------------*/

static void generateRelatedKeys(u8 Ka[]) {
    for (int i = 0; i < 16; i++) {
        if (i == 4)
            Kb[i] = Ka[i] ^ 0x80;
        else 
            Kb[i] = Ka[i];      
    }

    for (int i = 0; i < 16; i++) {
        if (i == 12)
            Kc[i] = Ka[i] ^ 0x80;
        else 
            Kc[i] = Ka[i];      
    }

    for (int i = 0; i < 16; i++) {
        if (i == 4)
            Kd[i] = Kc[i] ^ 0x80;
        else 
            Kd[i] = Kc[i];      
    }
}

int main(void) {
    time_t t;

    // Hardcoded key Ka
    Ka = (u8 [16]) {
        0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 
    };

    generateRelatedKeys(Ka);

    printHex("Ka", Ka, 16);
    printHex("Kb", Kb, 16);
    printHex("Kc", Kc, 16);
    printHex("Kd", Kd, 16);

    /*-------------------------------------------------------------------------------------------
     * 1. Data Collection Phase:
     *-------------------------------------------------------------------------------------------*/

    /*-------------------------------------------------------------------------------------------
     *      (a) ask for the encryption of 2^38 plaintexts pairs (P_a, P_b) such that
     *          P_a xor P_b = A = (0, 0, 0010_x, 0) with keys K_a, K_b
     *-------------------------------------------------------------------------------------------*/
    

    /* Intializes random number generator */
    srand((unsigned) time(&t));

    u8 Pa[8];
    for (int i = 0; i < 8; i++) {
        //Pa[i] = rand() % 255;
        Pa[i] = 0xff;
    }
    printHex("Pa", Pa, 8);

    u8 Pb[8];
    for (int i = 0; i < 8; i++) {
        if (i == 5)
            Pb[i] = Pa[i] ^ 0x10;
        else
            Pb[i] = Pa[i];
    }
    printHex("Pb", Pb, 8);
    
    u8 Ca[8];
    //for (int i = 0; i < 8; i++) Ca[i] = Pa[i];
    memcpy(Ca, &Pa[0], 8*sizeof(*Pa));
    KeySchedule(Ka);
    Kasumi(Ca);
    printHex("Ca", Ca, 8);

    u8 Cb[8];
    //for (int i = 0; i < 8; i++) Cb[i] = Pb[i];
    memcpy(Cb, &Pb[0], 8*sizeof(*Pb));
    KeySchedule(Kb);
    Kasumi(Cb);
    printHex("Cb", Cb, 8);

    /*-------------------------------------------------------------------------------------------
     *          TODO: inserting the plaintext pairs (P_a, P_b)
     *          into a hash table indexed by the values (C_a^RL, C_a^RR, C_b^RL, C_b^RR)
     *-------------------------------------------------------------------------------------------*/

    u8 hash[8];
    memcpy(hash, &Ca[0], 4*sizeof(*Ca));
    memcpy(hash+4, &Cb[0], 4*sizeof(*Cb));
    printHex("HASH", hash, 8);

    /*-------------------------------------------------------------------------------------------
     *      (b) ask for the encryption for other 2^38 plaintexts pairs (P_c, P_d) such that
     *          P_c xor P_d = A with keys K_c, K_d
     *-------------------------------------------------------------------------------------------*/

    u8 Pc[8];
    for (int i = 0; i < 8; i++) {
        //Pc[i] = rand() % 255;
        Pc[i] = 0x11;
    }
    printHex("Pc", Pc, 8);

    u8 Pd[8];
    for (int i = 0; i < 8; i++) {
        if (i == 5)
            Pd[i] = Pc[i] ^ 0x10;
        else
            Pd[i] = Pc[i];
    }
    printHex("Pd", Pd, 8);
    
    u8 Cc[8];
    //for (int i = 0; i < 8; i++) Cc[i] = Pc[i];
    memcpy(Cc, &Pc[0], 8*sizeof(*Pc));
    KeySchedule(Kc);
    Kasumi(Cc);
    printHex("Cc", Cc, 8);

    u8 Cd[8];
    //for (int i = 0; i < 8; i++) Cd[i] = Pd[i];
    memcpy(Cd, &Pd[0], 8*sizeof(*Pd));
    KeySchedule(Kd);
    Kasumi(Cd);
    printHex("Cd", Cd, 8);

    /*-------------------------------------------------------------------------------------------
     *          TODO: for each pair (P_c, P_d) the attacker accesses the hash table in the entry 
     *          corresponding to the value (C_c^RL xor 0010_x, C_c^RR, C_d^RL xor 0010_x, C_d^RR)
     *          and for each pair (P_a, P_b) stored in the entry constructs the quartet 
     *          (P_a, P_b, P_c, P_d).
     *-------------------------------------------------------------------------------------------*/
}