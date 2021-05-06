#include <stdio.h>

int main(){
    // int mat[10][10] = {0};
    // int conta = 0;
    // for(int i=0; i<10; i++){
    //     for(int j=0; j<10; j++){
    //         mat[i][j] = conta;
    //         conta++;
    //     }
    // }

    // printf("\n\n");

    // for(int i=0; i<10; i++){
    //     for(int j=0; j<10; j++){
    //         printf(" (%d) ", mat[i][j]);
    //     }
    //     printf("\n");
    // }

    // printf("\n\n");

    // for(int i=4; i<4+3; i++){
    //     for(int j=4; j<4+3; j++){
    //         printf(" %d ", mat[i][j]);
    //     }
    //     printf("\n");
    // }

    // printf("\n\n");

    // for(int i=4; i>4-3; i--){
    //     for(int j=4; j>4-3; j--){
    //         printf(" %d ", mat[i][j]);
    //     }
    //     printf("\n");
    // }

    // printf("\n\n");

    // for(int i=4-2; i<=4; i++){
    //     for(int j=4-2; j<=4; j++){
    //         printf(" %d ", mat[i][j]);
    //     }
    //     printf("\n");
    // }
    

    for(int i=0; i<10; i++){
        for(int j=0; j<10; j++){
            printf(" (%d, %d) ", i, j);
        }
        printf("\n");
    }

    printf("\n\n");

    int x=0;
    int y=0;
    int xinf = x-2;
    int xsup = x+3;
    int yinf = y-2;
    int ysup = y+3;
    if(xinf < 0){
        xinf = 0;
    }
    if(xsup > 10){
        xsup = 10;
    } 
    if(yinf < 0){
        yinf = 0;
    }
    if(ysup > 10){
        ysup = 10;
    }

    for(int i=xinf; i<xsup; i++){
        for(int j=yinf; j<ysup; j++){
            printf(" (%d, %d) ", i, j);
        }
        printf("\n");
    }


    return 0;
}