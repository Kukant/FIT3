
/* ******************************* c204.c *********************************** */
/*  Předmět: Algoritmy (IAL) - FIT VUT v Brně                                 */
/*  Úkol: c204 - Převod infixového výrazu na postfixový (s využitím c202)     */
/*  Referenční implementace: Petr Přikryl, listopad 1994                      */
/*  Přepis do jazyka C: Lukáš Maršík, prosinec 2012                           */
/*  Upravil: Kamil Jeřábek, říjen 2017                                        */
/* ************************************************************************** */
/*
** Implementujte proceduru pro převod infixového zápisu matematického
** výrazu do postfixového tvaru. Pro převod využijte zásobník (tStack),
** který byl implementován v rámci příkladu c202. Bez správného vyřešení
** příkladu c202 se o řešení tohoto příkladu nepokoušejte.
**
** Implementujte následující funkci:
**
**    infix2postfix .... konverzní funkce pro převod infixového výrazu
**                       na postfixový
**
** Pro lepší přehlednost kódu implementujte následující pomocné funkce:
**
**    untilLeftPar .... vyprázdnění zásobníku až po levou závorku
**    doOperation .... zpracování operátoru konvertovaného výrazu
**
** Své řešení účelně komentujte.
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako
** procedury (v jazyce C procedurám odpovídají funkce vracející typ void).
**
**/

#include "c204.h"

int solved;


/*
** Pomocná funkce untilLeftPar.
** Slouží k vyprázdnění zásobníku až po levou závorku, přičemž levá závorka
** bude také odstraněna. Pokud je zásobník prázdný, provádění funkce se ukončí.
**
** Operátory odstraňované ze zásobníku postupně vkládejte do výstupního pole
** znaků postExpr. Délka převedeného výrazu a též ukazatel na první volné
** místo, na které se má zapisovat, představuje parametr postLen.
**
** Aby se minimalizoval počet přístupů ke struktuře zásobníku, můžete zde
** nadeklarovat a používat pomocnou proměnnou typu char.
*/
void untilLeftPar ( tStack* s, char* postExpr, unsigned* postLen ) {
    char b;
    if (stackEmpty(s))
            return;
    // popneme prvni znak
    stackTop(s, &b);
    stackPop(s);
    // popujeme az dokud nenarazime na levou, overujeme jestli zasobnik neni prazdny
    while (b != '(')
    {
        postExpr[*postLen] = b;
        (*postLen)++;
        if (stackEmpty(s))
            return;

        stackTop(s, &b);
        stackPop(s);
    }
}

/*
** Pomocná funkce doOperation.
** Zpracuje operátor, který je předán parametrem c po načtení znaku ze
** vstupního pole znaků.
**
** Dle priority předaného operátoru a případně priority operátoru na
** vrcholu zásobníku rozhodneme o dalším postupu. Délka převedeného
** výrazu a taktéž ukazatel na první volné místo, do kterého se má zapisovat,
** představuje parametr postLen, výstupním polem znaků je opět postExpr.
*/
void doOperation ( tStack* s, char c, char* postExpr, unsigned* postLen ) {
    if ( c == '(')
    {
        stackPush(s, c);
    }
    else if (c == ')')
    {
        untilLeftPar(s, postExpr, postLen);
    }
    else // hlavni logika
    {
        int c_prior;
        int top_prior;
        char b;

        if (stackEmpty(s))
        {
            stackPush(s, c);
            return;
        }

        stackTop(s, &b);
	// zjistujeme prioritu
        switch (c){
            case '=':
                c_prior = -2;
                break;
            case '(':
                c_prior = -1;
                break;
            case '+':
            case '-':
                c_prior = 0;
                break;
            case '*':
            case '/':
                c_prior = 1;
                break;
        }

	// priorita pro znak v zasobniku
        switch (b){
            case '=':
                top_prior = -2;
                break;
            case '(':
                top_prior = -1;
                break;
            case '+':
            case '-':
                top_prior = 0;
                break;
            case '*':
            case '/':
                top_prior = 1;
                break;
        }
	// popujeme ze zasobniku v zavislosti na priorite
        while (top_prior >= c_prior && b != '(')
        {
            postExpr[*postLen] = b;
            (*postLen)++;

            stackPop(s);

            if (stackEmpty(s))
                break;

            stackTop(s, &b);


            switch (b){
                case '=':
                    top_prior = -2;
                    break;
                case '(':
                    top_prior = -1;
                    break;
                case '+':
                case '-':
                    top_prior = 0;
                    break;
                case '*':
                case '/':
                    top_prior = 1;
                    break;
            }
        }
        stackPush(s, c);
    }
}

/*
** Konverzní funkce infix2postfix.
** Čte infixový výraz ze vstupního řetězce infExpr a generuje
** odpovídající postfixový výraz do výstupního řetězce (postup převodu
** hledejte v přednáškách nebo ve studijní opoře). Paměť pro výstupní řetězec
** (o velikosti MAX_LEN) je třeba alokovat. Volající funkce, tedy
** příjemce konvertovaného řetězce, zajistí korektní uvolnění zde alokované
** paměti.
**
** Tvar výrazu:
** 1. Výraz obsahuje operátory + - * / ve významu sčítání, odčítání,
**    násobení a dělení. Sčítání má stejnou prioritu jako odčítání,
**    násobení má stejnou prioritu jako dělení. Priorita násobení je
**    větší než priorita sčítání. Všechny operátory jsou binární
**    (neuvažujte unární mínus).
**
** 2. Hodnoty ve výrazu jsou reprezentovány jednoznakovými identifikátory
**    a číslicemi - 0..9, a..z, A..Z (velikost písmen se rozlišuje).
**
** 3. Ve výrazu může být použit předem neurčený počet dvojic kulatých
**    závorek. Uvažujte, že vstupní výraz je zapsán správně (neošetřujte
**    chybné zadání výrazu).
**
** 4. Každý korektně zapsaný výraz (infixový i postfixový) musí být uzavřen
**    ukončovacím znakem '='.
**
** 5. Při stejné prioritě operátorů se výraz vyhodnocuje zleva doprava.
**
** Poznámky k implementaci
** -----------------------
** Jako zásobník použijte zásobník znaků tStack implementovaný v příkladu c202.
** Pro práci se zásobníkem pak používejte výhradně operace z jeho rozhraní.
**
** Při implementaci využijte pomocné funkce untilLeftPar a doOperation.
**
** Řetězcem (infixového a postfixového výrazu) je zde myšleno pole znaků typu
** char, jenž je korektně ukončeno nulovým znakem dle zvyklostí jazyka C.
**
** Na vstupu očekávejte pouze korektně zapsané a ukončené výrazy. Jejich délka
** nepřesáhne MAX_LEN-1 (MAX_LEN i s nulovým znakem) a tedy i výsledný výraz
** by se měl vejít do alokovaného pole. Po alokaci dynamické paměti si vždycky
** ověřte, že se alokace skutečně zdrařila. V případě chyby alokace vraťte namísto
** řetězce konstantu NULL.
*/
char* infix2postfix (const char* infExpr) {
    long len = 0;
    while(infExpr[len] != '\0')
        len++;

    tStack s;
    stackInit(&s);
    char *result = calloc(sizeof(char), len + 1);
    if (result == NULL)
        return NULL;
    if (len == 0)
        return result;


    // for each char
    unsigned i, j = 0;
    for ( i = 0; i < len; i++)
    {
        char c = infExpr[i];
        // if is operator
        if ( (c > 64 && c < 91) || (c > 96 && c < 123) || (c > 47 && c < 58))
            result[j++] = c;
        else
            doOperation (&s, c, result, &j);
    }
    // ze zasobniku vyplivneme zbytek
    while (stackEmpty(&s) == 0)
    {
        char b;
        stackTop(&s, &b);
        stackPop(&s);
        result[j++] = b;
    }

    return result;

}

/* Konec c204.c */
