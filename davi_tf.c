#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

int screenX=1024,screenY=860;

typedef struct{
    int x,y,arrL,arrC;
} Ponto;

typedef struct{
    int matriz[3][3],minutos,segundos,vitoria,modo,jogador;
} Partida;

typedef struct{
    int matriz[3][3],tempo,modo,jogador,next,fichasp0,fichasp1,turno;
} Save;

void jogarPartida(int modo,int vezJ,int tabuleiro[3][3],int fichas[2],int jogador,int tempo,int turno,ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,ALLEGRO_EVENT_QUEUE *queue,ALLEGRO_COLOR blue, ALLEGRO_COLOR red,int *exit,int *pvp,int *menu, ALLEGRO_TIMER *timer, Ponto tPts[7],int loaded);
void deleteSave(int modo);

int inRange(int n,int target,int r){
    if(n>=target-r && n<=target+r) return 1;
    return 0;
}

int isFileEmpty(FILE *arq){
    fseek(arq,0,SEEK_END);
    int final = ftell(arq);
    rewind(arq);
    return final==0;
}

int isValidMove(int iniL,int iniC,int fimL,int fimC,int tabuleiro[3][3]){
    if(tabuleiro[fimL][fimC]!=-1) return 0;
    iniC = (iniL==0)?fimC:iniC;
    fimC = (fimL==0)?iniC:fimC;
    if(iniL==fimL){
        if(abs(iniC-fimC)==1 && tabuleiro[fimL][fimC]==-1) return 1;
        if(abs(iniC-fimC)==2 && tabuleiro[fimL][(iniC+fimC)/2]!=-1) return 1;
    }
    if(iniC==fimC){
        if(abs(iniL-fimL)==1 && tabuleiro[fimL][fimC]==-1) return 1;
        if(abs(iniL-fimL)==2 && tabuleiro[(iniL+fimL)/2][fimC]!=-1) return 1;
    }
    return 0;
}

void eventExit(ALLEGRO_EVENT evento,int *rodando,int *current,int *extra,int *exit){
    if(evento.type==ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE || evento.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
        *rodando = 0;
        *current = 0;
        *extra = 0;
        *exit = 1;
    }
}

int randomBool(){
    return rand()%2;
}

void desenharBotao(ALLEGRO_FONT *fonte, int x1,int y1,int x2,int y2,int txtY,ALLEGRO_COLOR cor,char *texto){
    al_draw_filled_rounded_rectangle(x1,y1,x2,y2,10,10,cor);
    al_draw_rounded_rectangle(x1,y1,x2,y2,10,10,al_map_rgb(0,0,0),3);
    al_draw_text(fonte,al_map_rgb(0,0,0),(x1+x2)/2,txtY,ALLEGRO_ALIGN_CENTER,texto);
}

int getPartidas(Partida ptds[],int modo, FILE *arq,int *menorQtd,int *maiorQtd){
    int flag = 1,i=0,tempo;
    *menorQtd = 100000;
    *maiorQtd = 0;
    if(modo!=2) flag = 0;
    char linha[50];
    while(fgets(linha,50,arq)){
        int modoTmp;
        fgets(linha,sizeof(linha),arq);
        sscanf(linha,"modo %d",&modoTmp);
        if(modoTmp==modo || modo==2){
            ptds[i].modo = modoTmp;
            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"jogador %d",&ptds[i].jogador);
            for(int j=0;j<3;j++){
                fgets(linha,sizeof(linha),arq);
                sscanf(linha,"%d %d %d",&ptds[i].matriz[j][0],&ptds[i].matriz[j][1],&ptds[i].matriz[j][2]);
            }
            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"minutos %d",&ptds[i].minutos);

            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"segundos %d",&ptds[i].segundos);

            tempo = ptds[i].minutos*60 + ptds[i].segundos;
            *menorQtd = (*menorQtd>tempo)?tempo:*menorQtd;
            *maiorQtd = (*maiorQtd<tempo)?tempo:*maiorQtd;

            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"vitoria %d",&ptds[i].vitoria);

            fgets(linha,sizeof(linha),arq);
            i++;
        }else{
            for(int j=0;j<8;j++) fgets(linha,sizeof(linha),arq);
        }
    }
    return i;
}

int partidasArquivo(FILE *arq,int modo){
    if(arq==NULL) return 0;
    int linhas=0;
    char l[64];
    
    while(fgets(l, sizeof(l), arq)){
        l[strcspn(l, "\n")] = 0;
        if(strncmp(l,"<JOGO>",6)==0){
            if(modo!=2){
                int flag = 0;
                char cmp[8];
                fgets(l,sizeof(l),arq);
                l[strcspn(l, "\n")] = 0;
                sprintf(cmp,"modo %d",modo);
                if(strcmp(l,cmp)==0) flag = 1;
                if(flag) linhas++;
            }else{
                linhas++;
            }
        }
    }
    rewind(arq);
    return linhas;
}

void desenharFichas(int fichas[2],int tabuleiro[3][3],int jogador,Ponto *tPts,int selecionado,ALLEGRO_BITMAP *blueChip,ALLEGRO_BITMAP *redChip,int chipOffset){
    int i,ini[2] = {100,screenX/2+50},fim[2] = {screenX/2-50,screenX-100},iniY=600,fimY=700,l,c;
    ALLEGRO_BITMAP *chips[2];
    chips[0] = blueChip;
    chips[1] = redChip;
    
    if(selecionado == 8){
        if(fichas[0]==1){
            al_draw_filled_rounded_rectangle(screenX/2-360,290,screenX/2-260,390,5,5,al_map_rgb(122, 121, 121));
            al_draw_bitmap(chips[0],screenX/2-350,300,0);
        }
        if(fichas[1]==1){
            al_draw_filled_rounded_rectangle(screenX/2+290,290,screenX/2+390,390,5,5,al_map_rgb(122, 121, 121));
            al_draw_bitmap(chips[1],screenX/2+300,300,0);
        }
        selecionado = -1;
    }else{
        for(i=0;i<2;i++){
            if(fichas[i]>0){
                al_draw_filled_rounded_rectangle(ini[i],iniY,fim[i],fimY,20,20,al_map_rgb(122, 121, 121));
                for(int j=0;j<fichas[i];j++){
                    al_draw_bitmap(chips[i],ini[i]+50+(j*90),iniY+10,0);
                    if(j==fichas[i]-1 && jogador==i){
                        al_draw_circle(ini[i]+90+(j*90),iniY+50,35,al_map_rgb(255, 245, 54),3);
                    }
                }
            }
        }
    }


    for(i=0;i<7;i++){
        l = tPts[i].arrL,c = tPts[i].arrC;
        if(tabuleiro[l][c]!=-1){
            al_draw_bitmap(chips[tabuleiro[l][c]],tPts[i].x-chipOffset,tPts[i].y-chipOffset,0);
            if(selecionado == i){
                al_draw_circle(tPts[i].x,tPts[i].y,35,al_map_rgb(255, 245, 54),3);
            }
        }
    }
}

void desenharTabuleiro(Ponto *tPts,int yOffConst){
    int lineOffsetY = (yOffConst*sqrt(3)/4)+tPts[0].y;
    int raio = (yOffConst==500)?10:7;
    al_draw_triangle(tPts[0].x,tPts[0].y,tPts[4].x,tPts[4].y,tPts[6].x,tPts[6].y,al_map_rgb(0,0,0),4);

    al_draw_line(tPts[0].x,tPts[0].y,tPts[5].x,tPts[5].y,al_map_rgb(0,0,0),4);
    al_draw_line(tPts[1].x,lineOffsetY,tPts[3].x,lineOffsetY,al_map_rgb(0,0,0),4);

    for(int i=0;i<7;i++){
        al_draw_filled_circle(tPts[i].x,tPts[i].y,raio,al_map_rgb(166, 179, 176));
        al_draw_circle(tPts[i].x,tPts[i].y,raio,al_map_rgb(0,0,0),2);
    }
}

void desenharCaminho(float x1,float y1,float x2,float y2,ALLEGRO_COLOR cor){
    float dx = x1 - x2;
    float dy = y1 - y2;
    float length = sqrtf(dx * dx + dy * dy);
    float ux = dx / length;
    float uy = dy / length;
    al_draw_line(x1,y1,x2+ux*15,y2+uy*15,cor,8);
    al_draw_filled_circle(x2+ux*15,y2+uy*15,7,cor);
    al_draw_circle(x2+ux*15,y2+uy*15,5,al_map_rgb(0,0,0),2);
}

void checkDrawBetween(int i,int j,Ponto *tPts,int tabuleiro[3][3],ALLEGRO_COLOR cor){
    if(i!=j && isValidMove(tPts[i].arrL,tPts[i].arrC,tPts[j].arrL,tPts[j].arrC,tabuleiro)){
        desenharCaminho(tPts[i].x,tPts[i].y,tPts[j].x,tPts[j].y,cor);
    }
}

void mostrarMovimentos(int jogador,int tabuleiro[3][3],Ponto *tPts,int select,ALLEGRO_COLOR cor[2]){
    int i;
    for(i=0;i<7;i++){
        if(select!=-1){
            checkDrawBetween(select,i,tPts,tabuleiro,cor[jogador]);
        }else{
            if(tabuleiro[tPts[i].arrL][tPts[i].arrC]!=jogador) continue;
            for(int j=0;j<7;j++){
                checkDrawBetween(i,j,tPts,tabuleiro,cor[jogador]);
            }
        }
    }
}

void desenharTimer(ALLEGRO_FONT *medfont,int minutos,int segundos){
    char text[10];
    al_draw_filled_rounded_rectangle(30,15,160,55,10,10,al_map_rgb(255, 228, 169));
    sprintf(text,"%02d:%02d",minutos,segundos);
    al_draw_text(medfont,al_map_rgb(0,0,0),95,25,ALLEGRO_ALIGN_CENTER,text);
}

void interfacePartida(ALLEGRO_BITMAP *partidabg,ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,int minutos,int segundos,Ponto tPts[7],int jogador,int fichas[2],int tabuleiro[3][3],int selecionado,ALLEGRO_COLOR cores[2],int turno,int p1,int modo,ALLEGRO_BITMAP *blueChip,ALLEGRO_BITMAP *redChip){
    char text[30];
    al_draw_bitmap(partidabg,0,0,0);
    desenharBotao(medfont,screenX/2-250,740,screenX/2-50,800,760,al_map_rgb(145,209,227),"Pausar");
    desenharBotao(medfont,screenX/2+50,740,screenX/2+250,800,760,al_map_rgb(145,209,227),"Salvar");
    desenharTimer(medfont,minutos,segundos);
    desenharTabuleiro(tPts,500);
    if(fichas[jogador]==0) mostrarMovimentos(jogador,tabuleiro,tPts,selecionado,cores);
    desenharFichas(fichas,tabuleiro,jogador,tPts,selecionado,blueChip,redChip,40);
    sprintf(text,"Turno %d",turno);
    al_draw_textf(bigfont,al_map_rgb(128,146,142),screenX/2,30,ALLEGRO_ALIGN_CENTER,text);
    int position = (jogador==0)?screenX/4:screenX/4*3;
    char msg1[20],msg2[20];
    if(modo==0){
        sprintf(msg1,"Jogador 1");
        sprintf(msg2,"Jogador 2");
    }else{
        sprintf(msg1,"%s",(p1==0)?"Jogador":"Computador");
        sprintf(msg2,"%s",(p1==0)?"Computador":"Jogador");
    }
    sprintf(text,"Vez do %s",(jogador)?msg2:msg1);
    al_draw_text(medfont,cores[jogador],position,90,ALLEGRO_ALIGN_CENTER,text);
}

int checkVitoria(int tabuleiro[3][3]){
    int p,i,j,flag=0;
    for(p=0;p<2;p++){
        if(tabuleiro[0][1]==p){
            for(j=0;j<3;j++){
                if(tabuleiro[1][j]==p && tabuleiro[2][j]==p) return p;
            }
        }
        if(tabuleiro[1][1]==p){
            if(tabuleiro[1][0]==p && tabuleiro[1][2]==p) return p;
        }
        if(tabuleiro[2][1]==p){
            if(tabuleiro[2][0]==p && tabuleiro[2][2]==p) return p;
        }
    }
    return -1;
}

void salvarJogo(int modo,int vezJ,int tabuleiro[3][3],int fichas[2],int jogador,int tempo,int turno){
    FILE *arq = fopen("saves.txt","r+");
    FILE *tmp = fopen("tempsaves.txt","w");
    char linha[30],header[20],headerGenerica[20];
    int encontrou=0,ignorar=0;
    sprintf(headerGenerica,">SAVE modo ");
    sprintf(header,">SAVE modo %d",modo);
    if(arq){
        while(fgets(linha,sizeof(linha),arq)){
            if(strncmp(linha,headerGenerica,strlen(headerGenerica))==0){
                if(strncmp(linha,header,strlen(header))==0){
                    encontrou = 1;
                    ignorar = 1;
                    fprintf(tmp,"%s\n",header);
                    fprintf(tmp,"vezJ %d\n",vezJ);
                    for(int i=0;i<3;i++){
                        fprintf(tmp,"%d %d %d\n",tabuleiro[i][0],tabuleiro[i][1],tabuleiro[i][2]);
                    }
                    fprintf(tmp,"fichasp0 %d\n",fichas[0]);
                    fprintf(tmp,"fichasp1 %d\n",fichas[1]);
                    fprintf(tmp,"next %d\n",jogador);
                    fprintf(tmp,"tempo %d\n",tempo);
                    fprintf(tmp,"turno %d\n",turno);
                }else{
                    ignorar = 0;
                    fputs(linha,tmp);
                }
            }else if(!ignorar){
                fputs(linha,tmp);
            }
        }
        fclose(arq);
        remove("saves.txt");
    }

    if(!encontrou){
        fprintf(tmp,"%s\n",header);
        fprintf(tmp,"vezJ %d\n",vezJ);
        for(int i=0;i<3;i++){
            fprintf(tmp,"%d %d %d\n",tabuleiro[i][0],tabuleiro[i][1],tabuleiro[i][2]);
        }
        fprintf(tmp,"fichasp0 %d\n",fichas[0]);
        fprintf(tmp,"fichasp1 %d\n",fichas[1]);
        fprintf(tmp,"next %d\n",jogador);
        fprintf(tmp,"tempo %d\n",tempo);
        fprintf(tmp,"turno %d\n",turno);
    }
    fclose(tmp);
    rename("tempsaves.txt","saves.txt");
}

void desenharLinhaVitoria(Ponto tPts[7],int tabuleiro[3][3],ALLEGRO_COLOR cor,int jogador,int grossura){
    int pts[3][2],i,ind=0;
    for(i=0;i<7;i++){
        if(tabuleiro[tPts[i].arrL][tPts[i].arrC]==jogador){
            pts[ind][0] = tPts[i].x;
            pts[ind][1] = tPts[i].y;
            ind++;
        }
    }
    al_draw_line(pts[0][0],pts[0][1],pts[1][0],pts[1][1],cor,grossura);
    al_draw_line(pts[1][0],pts[1][1],pts[2][0],pts[2][1],cor,grossura);
}

void displayPartida(int yOff,Partida ptds[3],int i,ALLEGRO_FONT *smallfont,ALLEGRO_COLOR blue,ALLEGRO_COLOR red,ALLEGRO_BITMAP *blueChip,ALLEGRO_BITMAP *redChip){
    int triMeio = screenX/2-160,triOff=60;
    Ponto pts[7];
    pts[0]=(Ponto){triMeio,20+yOff,0,1};
    pts[1]=(Ponto){triMeio-(triOff/2),70+yOff,1,0};
    pts[2]=(Ponto){triMeio,70+yOff,1,1};
    pts[3]=(Ponto){triMeio+(triOff/2),70+yOff,1,2};
    pts[4]=(Ponto){triMeio-triOff,120+yOff,2,0};
    pts[5]=(Ponto){triMeio,120+yOff,2,1};
    pts[6]=(Ponto){triMeio+triOff,120+yOff,2,2};
    desenharTabuleiro(pts,120);
    int fichas[2] = {0,0};
    if(ptds[i].vitoria!=-1){
        desenharLinhaVitoria(pts,ptds[i].matriz,(ptds[i].vitoria)?red:blue,ptds[i].vitoria,5);
    }
    desenharFichas(fichas,ptds[i].matriz,ptds[i].vitoria,pts,-1,blueChip,redChip,10);

    char texto[20],vencedor[20];
    ALLEGRO_COLOR corVenc;
    if(ptds[i].vitoria==-1){
        sprintf(vencedor,"EMPATE");
        corVenc = al_map_rgb(0,0,0);
    }else{
        if(ptds[i].modo==1){
            sprintf(vencedor,"%s",(ptds[i].vitoria==ptds[i].jogador)?"Jogador":"Computador");
        }else sprintf(vencedor,"%s",(ptds[i].vitoria)?"Vermelho":"Azul");
        corVenc = (ptds[i].vitoria)?red:blue;
    }
    sprintf(texto,"Vencedor: %s",vencedor);
    al_draw_text(smallfont,corVenc,screenX/2-86,20+yOff,0,texto);
    sprintf(texto,"Tempo: %02d:%02d",ptds[i].minutos,ptds[i].segundos);
    al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2-85,20+40+yOff,0,texto);
    sprintf(texto,"Modo: %s",(ptds[i].modo==0)?"PvP":"PvC");
    al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2-82,20+80+yOff,0,texto);
}

void victoryScreen(int *pvp,int *pvprodando,int *menu,int *exit,int tabuleiro[3][3],int modo,ALLEGRO_TIMER *timer,int turnos,Ponto tPts[7],ALLEGRO_COLOR cor,int *fichas,int jogador,ALLEGRO_EVENT_QUEUE *queue,ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,int vezJ,int loaded){
    int rodando = 1,seconds = al_get_timer_count(timer)*al_get_timer_speed(timer),minutos = seconds/60,segundos = seconds%60;
    char text[50],vitorioso[20];
    ALLEGRO_BITMAP *blueChip = al_load_bitmap("assets/blueChip.png"),*redChip = al_load_bitmap("assets/redChip.png");

    FILE *arq = fopen("historico.txt","a");
    fprintf(arq,"<JOGO>\n");
    fprintf(arq,"modo %d\n",modo);
    fprintf(arq,"jogador %d\n",vezJ);
    for(int i=0;i<3;i++)
        fprintf(arq,"%d %d %d\n",tabuleiro[i][0],tabuleiro[i][1],tabuleiro[i][2]);
    fprintf(arq,"minutos %d\n",minutos);
    fprintf(arq,"segundos %d\n",segundos);
    fprintf(arq,"vitoria %d\n",jogador);
    fprintf(arq,"</JOGO>\n");
    fclose(arq);

    while(rodando){
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue,&evento);

        eventExit(evento,&rodando,pvprodando,pvp,exit);
        al_clear_to_color(al_map_rgb(166, 179, 176));

        desenharTabuleiro(tPts,500);

        if(jogador == -1){
            al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,50,ALLEGRO_ALIGN_CENTER,"Empate");
        }else{
            if(modo==0){
                sprintf(vitorioso,"Jogador %d",jogador+1);
            }else sprintf(vitorioso,"%s",(jogador==vezJ)?"Jogador":"Computador");
            sprintf(text,"Vitória do %s!",vitorioso);
            al_draw_text(bigfont,cor,screenX/2,50,ALLEGRO_ALIGN_CENTER,text);
            desenharLinhaVitoria(tPts,tabuleiro,cor,jogador,8);
        }
        desenharFichas(fichas,tabuleiro,jogador,tPts,8,blueChip,redChip,40);

        if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
            if(evento.mouse.y>=720 && evento.mouse.y<=800){
                if(evento.mouse.x>=screenX/2-385 && evento.mouse.x<=screenX/2-15){
                    *pvp = 1;
                    al_set_timer_count(timer,0);
                    rodando = 0;
                    *pvprodando = 0;
                }
                if(evento.mouse.x>=screenX/2+15 && evento.mouse.x<=screenX/2+385){
                    *pvp = 0;
                    rodando = 0;
                    *pvprodando = 0;
                    *menu = 1;
                }
            }
        }

        sprintf(text,"Tempo de partida: %.02d:%.02d",minutos,segundos);
        al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2,640,ALLEGRO_ALIGN_CENTER,text);
        sprintf(text,"Turnos totais: %d",turnos);
        al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2,680,ALLEGRO_ALIGN_CENTER,text);
        
        desenharBotao(medfont,screenX/2-385,720,screenX/2-15,800,750,al_map_rgb(145,209,227),"Jogar novamente");
        desenharBotao(medfont,screenX/2+15,720,screenX/2+385,800,750,al_map_rgb(145,209,227),"Voltar ao menu");

        al_flip_display();
    }
    al_destroy_bitmap(blueChip);
    al_destroy_bitmap(redChip);
}

Save getSave(int modo){
    Save save;
    FILE *arq = fopen("saves.txt","r");
    char linha[30];
    char header[20];
    sprintf(header, ">SAVE modo %d", modo);

    while (fgets(linha, sizeof(linha), arq)){
        if (strncmp(linha, header, strlen(header)) == 0) {
            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"vezJ %d",&save.jogador);
            for(int i=0;i<3;i++){
                fgets(linha,sizeof(linha),arq);
                sscanf(linha,"%d %d %d",&save.matriz[i][0],&save.matriz[i][1],&save.matriz[i][2]);
            }
            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"fichasp0 %d",&save.fichasp0);
            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"fichasp1 %d",&save.fichasp1);
            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"next %d",&save.next);
            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"tempo %d",&save.tempo);
            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"turno %d",&save.turno);
            break;
        }
    }
    fclose(arq);
    return save;
}

void deleteSave(int modo){
    FILE *arq = fopen("saves.txt","r");
    if(!arq) return;
    FILE *tmp = fopen("savestmp.txt","w");
    int pular = 1;
    char linha[30];
    while(fgets(linha,sizeof(linha),arq)){
        if(strncmp(linha,">SAVE modo ",11)==0){
            int lerModo;
            sscanf(linha,">SAVE modo %d",&lerModo);
            pular = (lerModo==modo)?1:0;
        }
        
        if(!pular){
            fputs(linha,tmp);
        }
    }
    fclose(arq);
    fclose(tmp);
    remove("saves.txt");
    rename("savestmp.txt","saves.txt");
}

void menuScreen(ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,int yOff){
    ALLEGRO_BITMAP *menubg = al_load_bitmap("assets/menuBG.png"); 
    al_draw_bitmap(menubg,0,0,0);
    
    al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,yOff,ALLEGRO_ALIGN_CENTER,"TRI-Angle");

    desenharBotao(medfont,screenX/2-150,yOff+100,screenX/2+150,yOff+160,yOff+120,al_map_rgb(145,209,227),"Jogar");
    desenharBotao(medfont,screenX/2-150,yOff+200,screenX/2+150,yOff+260,yOff+220,al_map_rgb(145,209,227),"Como jogar");
    desenharBotao(medfont,screenX/2-150,yOff+300,screenX/2+150,yOff+360,yOff+320,al_map_rgb(145,209,227),"Histórico");
    desenharBotao(medfont,screenX/2-150,yOff+400,screenX/2+150,yOff+460,yOff+420,al_map_rgb(145,209,227),"Sair");
    al_destroy_bitmap(menubg);
}

void selectGamemodeScreen(ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,ALLEGRO_FONT *smallfont,ALLEGRO_TIMER *timer,Ponto tPts[7],int *pvp,int *pvc,int *jogar,int *menu,int *exit,ALLEGRO_COLOR blue, ALLEGRO_COLOR red,ALLEGRO_COLOR buttonColor,ALLEGRO_EVENT_QUEUE *queue){
    ALLEGRO_BITMAP *iconPvP = al_load_bitmap("assets/icon_pvp.png");
    ALLEGRO_BITMAP *iconPvC = al_load_bitmap("assets/icon_pc.png");
    ALLEGRO_TIMER *tick = al_create_timer(1.0/60.0);
    al_start_timer(tick);
    int rodando=1,redraw = 1,savePvP=0,savePvC=0;
    char line[30];
    FILE *arq = fopen("saves.txt","r");
    if(!arq){
        savePvP = 0;
        savePvC = 0;
    }else{
        while(fgets(line,sizeof(line),arq)){
            if(strncmp(line,">SAVE modo 0",12)==0) savePvP = 1;
            if(strncmp(line,">SAVE modo 1",12)==0) savePvC = 1;
        }
        fclose(arq);
    }
    int offPvP=screenX/4-200, offPvC=((screenX/4)*3)-200;
    int offY=150, endPvPY=(savePvP)?650:750,endPvCY=(savePvC)?650:750;
    while(rodando){
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue,&evento);

        eventExit(evento,&rodando,jogar,jogar,exit);

        if(evento.type==ALLEGRO_EVENT_TIMER) redraw = 1;

        if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
            if(evento.mouse.y>=offY){
                if(inRange(evento.mouse.x,offPvP+200,200) && evento.mouse.y<=endPvPY){
                    *pvp = 1;
                    *jogar = 0;
                    rodando = 0;
                }
                if(inRange(evento.mouse.x,offPvC+200,200) && evento.mouse.y<=endPvCY){
                    *pvc = 1;
                    *jogar = 0;
                    rodando = 0;
                }
            }
            redraw=1;
            int saveBools[2] = {savePvP,savePvC},offsets[2] = {offPvP,offPvC},endY[2] = {endPvPY,endPvCY};
            for(int i=0;i<2;i++){
                if(saveBools[i]){
                    if(inRange(evento.mouse.x,offsets[i]+200,200)){
                        if(inRange(evento.mouse.y,endY[i]+50,30)){
                            al_stop_timer(timer);
                            al_set_timer_count(timer,0);
                            Save partida = getSave(i);
                            int fichas[2] = {partida.fichasp0,partida.fichasp1};
                            if(i==0){
                                jogarPartida(0,0,partida.matriz,fichas,partida.next,partida.tempo,partida.turno,bigfont,medfont,queue,blue,red,exit,pvp,menu,timer,tPts,1);
                            }else{
                                jogarPartida(1,partida.jogador,partida.matriz,fichas,partida.next,partida.tempo,partida.turno,bigfont,medfont,queue,blue,red,exit,pvc,menu,timer,tPts,1);
                            }
                            *jogar = 0;
                            rodando = 0;
                            redraw = 1;
                        }
                        if(inRange(evento.mouse.y,endY[i]+130,30)){
                            deleteSave(i);
                            rodando = 0;
                        }
                    }
                }
            }
        }

        if(redraw){
            al_clear_to_color(al_map_rgb(166, 179, 176));

            al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2,50,ALLEGRO_ALIGN_CENTER,"Selecionar Modo de Jogo");

            al_draw_filled_rectangle((screenX/4)-200,offY,(screenX/4),endPvPY,blue);
            al_draw_filled_rectangle((screenX/4),offY,(screenX/4)+200,endPvPY,red);
            al_draw_rectangle((screenX/4)-200,offY,(screenX/4)+200,endPvPY,al_map_rgb(0,0,0),4);
            al_draw_bitmap(iconPvP,screenX/4-100,265,0);
            al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/4,550,ALLEGRO_ALIGN_CENTER,"PvP");
            al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/4,590,ALLEGRO_ALIGN_CENTER,"Player vs Player");
            if(savePvP){
                al_draw_filled_rectangle(screenX/4-200,endPvPY+20,screenX/4+200,endPvPY+80,buttonColor);
                al_draw_rectangle(screenX/4-200,endPvPY+20,screenX/4+200,endPvPY+80,al_map_rgb(0,0,0),4);
                al_draw_text(medfont,al_map_rgb(0,0,0),screenX/4,endPvPY+40,ALLEGRO_ALIGN_CENTER,"Continuar");

                al_draw_filled_rectangle(screenX/4-200,endPvPY+100,screenX/4+200,endPvPY+160,al_map_rgb(255, 77, 61));
                al_draw_rectangle(screenX/4-200,endPvPY+100,screenX/4+200,endPvPY+160,al_map_rgb(0,0,0),4);
                al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/4,endPvPY+120,ALLEGRO_ALIGN_CENTER,"Apagar jogo salvo");
            }
            if(savePvC){
                al_draw_filled_rectangle(screenX/4*3-200,endPvCY+20,screenX/4*3+200,endPvCY+80,buttonColor);
                al_draw_rectangle(screenX/4*3-200,endPvCY+20,screenX/4*3+200,endPvCY+80,al_map_rgb(0,0,0),4);
                al_draw_text(medfont,al_map_rgb(0,0,0),screenX/4*3,endPvCY+40,ALLEGRO_ALIGN_CENTER,"Continuar");

                al_draw_filled_rectangle(screenX/4*3-200,endPvCY+100,screenX/4*3+200,endPvCY+160,al_map_rgb(255, 77, 61));
                al_draw_rectangle(screenX/4*3-200,endPvCY+100,screenX/4*3+200,endPvCY+160,al_map_rgb(0,0,0),4);
                al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/4*3,endPvCY+120,ALLEGRO_ALIGN_CENTER,"Apagar jogo salvo");
            }

            int offsetX = screenX/4*3;
            al_draw_filled_rectangle(offsetX-200,offY,offsetX+200,endPvCY,blue);
            al_draw_rectangle(offsetX-200,offY,offsetX+200,endPvCY,al_map_rgb(0,0,0),4);
            al_draw_bitmap(iconPvC,offsetX-100,265,0);
            al_draw_text(bigfont,al_map_rgb(0,0,0),offsetX,550,ALLEGRO_ALIGN_CENTER,"PvC");
            al_draw_text(smallfont,al_map_rgb(0,0,0),offsetX,590,ALLEGRO_ALIGN_CENTER,"Player vs. Computer");

            redraw = 0;
            al_flip_display();
        }
    }

    al_stop_timer(tick);
    al_destroy_timer(tick);
    al_destroy_bitmap(iconPvP);
    al_destroy_bitmap(iconPvC);
}

void pauseScreen(ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,int *pvx, int *pvxrodando,int tempo,Ponto tPts[7],int jogador,int fichas[2],int tabuleiro[3][3],int turno,int modo,int vezJ,ALLEGRO_EVENT_QUEUE *queue,int *menu,int *exit){
    int rodando = 1,redraw = 1,yOff = 160,save=0;
    ALLEGRO_TIMER *tick = al_create_timer(1.0/60.0);
    al_start_timer(tick);
    while(rodando){
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue,&evento);

        if(evento.type==ALLEGRO_EVENT_TIMER) redraw = 1;

        eventExit(evento,&rodando,pvx,pvxrodando,exit);

        if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
            if(inRange(evento.mouse.x,screenX/2,150)){
                if(inRange(evento.mouse.y,yOff+30,30)) rodando = 0;
                if(inRange(evento.mouse.y,yOff*2+30,30)) {salvarJogo(modo,vezJ,tabuleiro,fichas,jogador,tempo,turno); save = 1; redraw = 1;}
                if(inRange(evento.mouse.y,yOff*3+30,30)) {*menu = 1; *pvx = 0; rodando = 0; *pvxrodando = 0;}
                if(inRange(evento.mouse.y,yOff*4+30,30)) {*exit = 1; *pvx = 0; rodando = 0; *pvxrodando = 0;}
            }
        }

        if(redraw){
            al_clear_to_color(al_map_rgb(166, 179, 176));

            al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,70,ALLEGRO_ALIGN_CENTER,"Jogo Pausado");

            desenharBotao(medfont,screenX/2-180,yOff,screenX/2+180,yOff+60,yOff+20,al_map_rgb(145, 209, 227),"Continuar");
            desenharBotao(medfont,screenX/2-180,yOff*2,screenX/2+180,yOff*2+60,yOff*2+20,al_map_rgb(145, 209, 227),"Salvar");
            desenharBotao(medfont,screenX/2-180,yOff*3,screenX/2+180,yOff*3+60,yOff*3+20,al_map_rgb(145, 209, 227),"Menu Principal");
            desenharBotao(medfont,screenX/2-180,yOff*4,screenX/2+180,yOff*4+60,yOff*4+20,al_map_rgb(145, 209, 227),"Sair");
            
            al_flip_display();
            redraw = 0;
            if(save==1){
                save = 0;
                al_draw_text(medfont,al_map_rgb(0, 255, 26),screenX-20,20,ALLEGRO_ALIGN_RIGHT,"Jogo Salvo!");
                al_flip_display();
                al_rest(1.0);
                redraw=1;
            }
        }
    }
    al_stop_timer(tick);
    al_destroy_timer(tick);
}

void gameAction(ALLEGRO_EVENT evento,int fichas[2],int tabuleiro[3][3],Ponto tPts[7],int *jogador,int *turno,int *selecionado){
    for(int i=0;i<7;i++){
        if(inRange(evento.mouse.x,tPts[i].x,20) && inRange(evento.mouse.y,tPts[i].y,20)){
            if(fichas[*jogador]>0 && tabuleiro[tPts[i].arrL][tPts[i].arrC] == -1){
                fichas[*jogador]--;
                tabuleiro[tPts[i].arrL][tPts[i].arrC] = *jogador;
                *jogador = !(*jogador);
                (*turno)++;
            }
            if(fichas[*jogador]==0){
                if(tabuleiro[tPts[i].arrL][tPts[i].arrC] == *jogador){
                    *selecionado = i;
                }
                if(*selecionado!=-1){
                    if(isValidMove(tPts[*selecionado].arrL,tPts[*selecionado].arrC,tPts[i].arrL,tPts[i].arrC,tabuleiro)){
                        tabuleiro[tPts[*selecionado].arrL][tPts[*selecionado].arrC] = -1;
                        tabuleiro[tPts[i].arrL][tPts[i].arrC] = *jogador;
                        *jogador = !(*jogador);
                        (*turno)++;
                        *selecionado = -1;
                    }
                }
            }
        }
    }
}

int verificarAlinhamento(int l,int c,int tabuleiro[3][3],int jogador,int considerarBloqueado){
    int align = 0,inimigo = !jogador,condicao;
    int adj[3][2] = {
        {1,2},
        {0,2},
        {0,1}
    };
    if(considerarBloqueado) condicao = 1;
    if(l==0){
        for(int j=0;j<3;j++){
            if(!considerarBloqueado) condicao = (tabuleiro[1][j]!=inimigo && tabuleiro[2][j]!=inimigo)?1:0;
            if((tabuleiro[1][j]==jogador || tabuleiro[2][j]==jogador) && condicao) align++;
        }
    }else{
        if(!considerarBloqueado) condicao = (tabuleiro[l][adj[c][0]]!=inimigo && tabuleiro[l][adj[c][1]]!=inimigo)?1:0;
        if(condicao && (tabuleiro[l][adj[c][0]]==jogador || tabuleiro[l][adj[c][1]]==jogador)) align++;

        if(!considerarBloqueado) condicao = (tabuleiro[adj[l][0]][c]!=inimigo && tabuleiro[adj[l][1]][c]!=inimigo)?1:0;
        if(condicao && (tabuleiro[adj[l][0]][c]==jogador || tabuleiro[adj[l][1]][c]==jogador)) align++;
    }
    return align;
}

int verificarWinCon(int l,int c,int tabuleiro[3][3],int jogador){
    int winCon=0;
    if(l==0){
        for(int i=0;i<3;i++) if(tabuleiro[l+1][i]==jogador && tabuleiro[l+2][i]==jogador) winCon++;
    }
    if(l==1 && tabuleiro[0][c]==jogador && tabuleiro[2][c]==jogador) winCon++;
    if(l==2 && tabuleiro[0][c]==jogador && tabuleiro[1][c]==jogador) winCon++;

    if(c==0 && tabuleiro[l][1]==jogador && tabuleiro[l][2]==jogador) winCon++;
    if(c==1 && tabuleiro[l][0]==jogador && tabuleiro[l][2]==jogador) winCon++;
    if(c==2 && tabuleiro[l][0]==jogador && tabuleiro[l][1]==jogador) winCon++;
    return winCon;
}

int verificaValorMovimento(int l, int c, int el, int ec, int tabTemp[3][3],int jogador){
    int tempPts = 50;
    tabTemp[el][ec] = tabTemp[l][c];
    tabTemp[l][c] = -1;
    tempPts += 50 * verificarAlinhamento(el,ec,tabTemp,jogador,1);
    if(checkVitoria(tabTemp)==jogador) return 400;
    if(verificarWinCon(l,c,tabTemp,!jogador)>0) return 15;
    return tempPts;
}

int escolhaPorPeso(int pesos[7],int qtd){
    int somaPesos=0;
    for(int i=0;i<qtd;i++) somaPesos+=pesos[i];

    if(somaPesos==0) return -1;
    int sel = rand()%somaPesos;
    int acc = 0;
    for(int i=0;i<qtd;i++){
        acc += pesos[i];
        if(sel<acc) return i;
    }
}

void computerAction(int fichas[2],int tabuleiro[3][3],Ponto tPts[7],int jogador,int turno){
    int pesos[7],free=-1;
    tabuleiro[0][0] = tabuleiro[0][1];
    tabuleiro[0][2] = tabuleiro[0][1];
    for(int i=0;i<7;i++){
        int l = tPts[i].arrL,c = tPts[i].arrC,condicao;
        pesos[i]=0;
        if(fichas[jogador]>0){
            condicao = -1;
            if(tabuleiro[l][c]==-1){
                int alinhamento = verificarAlinhamento(l,c,tabuleiro,jogador,0);
                pesos[i] += alinhamento*50;
                
                int winConInimigo = verificarWinCon(l,c,tabuleiro,!jogador);
                pesos[i] += winConInimigo*500;

                int winCon = verificarWinCon(l,c,tabuleiro,jogador);
                pesos[i] += winCon*4000;

                if(pesos[i]==0) pesos[i]=1;
            }
        }else{
            condicao = jogador;
            if(tabuleiro[0][1]==-1){
                free = 0;
            }else{
                for(int i=1;i<7;i++) if(tabuleiro[tPts[i].arrL][tPts[i].arrC]==-1) free = i;
            }
            int el = tPts[free].arrL,ec = tPts[free].arrC;
            int tabTemp[3][3];
            for(int j=0;j<3;j++){
                for(int k=0;k<3;k++) tabTemp[j][k] = tabuleiro[j][k];
            }

            if(tabuleiro[l][c]==jogador && isValidMove(l,c,el,ec,tabuleiro)){
                pesos[i] = verificaValorMovimento(l,c,el,ec,tabTemp,jogador);
                if(pesos[i]==0) pesos[i] = 1;
            }
        }
    }
    int sel = escolhaPorPeso(pesos,7);
    if(fichas[jogador]==0){
        tabuleiro[tPts[free].arrL][tPts[free].arrC] = jogador;
        tabuleiro[tPts[sel].arrL][tPts[sel].arrC] = -1;
    }else{
        tabuleiro[tPts[sel].arrL][tPts[sel].arrC] = jogador;
        fichas[jogador]--;
    }
    
    tabuleiro[0][0] = -1;
    tabuleiro[0][2] = -1;
}

void jogarPartida(int modo,int vezJ, int tabuleiro[3][3],int fichas[2],int jogador, int tempo, int turno,ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,ALLEGRO_EVENT_QUEUE *queue,ALLEGRO_COLOR blue, ALLEGRO_COLOR red,int *exit,int *pvx,int *menu, ALLEGRO_TIMER *timer, Ponto tPts[7], int loaded){
    int rodando=1,selecionado=-1,redraw=1,timerRedraw,pause=0,save=0,primeiraRodada = 1,vitoria = 0;
    char text[50];
    ALLEGRO_COLOR cores[] = {blue,red};
    ALLEGRO_BITMAP *partidabg = al_load_bitmap("assets/casteloBG.png"), *blueChip = al_load_bitmap("assets/blueChip.png"),*redChip = al_load_bitmap("assets/redChip.png");
    
    al_set_timer_count(timer,(int64_t)tempo/al_get_timer_speed(timer));
    if(modo==0 || (modo==1 && tempo>0)) al_resume_timer(timer);
    timerRedraw = (modo==0)?1:0;
    int segundosTotal,minutos,segundos;
    while(rodando){
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue,&evento);

        eventExit(evento,&rodando,pvx,pvx,exit);

        if(evento.type == ALLEGRO_EVENT_TIMER){
            timerRedraw = 1;
            segundosTotal = al_get_timer_count(timer) * al_get_timer_speed(timer);
            minutos = segundosTotal/60;
            segundos = segundosTotal%60;
        }
        
        if(modo == 0 || vezJ != -1){
            vitoria = checkVitoria(tabuleiro);
            if(vitoria!=-1){
                al_stop_timer(timer);
                victoryScreen(pvx,&rodando,menu,exit,tabuleiro,modo,timer,turno,tPts,cores[vitoria],fichas,vitoria,queue,bigfont,medfont,vezJ,loaded);
                redraw = 0;
            }
        }

        if(turno>49){
            al_stop_timer(timer);
            victoryScreen(pvx,&rodando,menu,exit,tabuleiro,modo,timer,turno,tPts,al_map_rgb(0,0,0),fichas,vitoria,queue,bigfont,medfont,vezJ,loaded);
            redraw = 0;
        }

        if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
            if(inRange(evento.mouse.x,screenX/2-150,50) && inRange(evento.mouse.y,770,30)){
                pause = 1;
                redraw = 1;
            }
            if(inRange(evento.mouse.x,screenX/2+150,50) && inRange(evento.mouse.y,770,30)){
                save = 1;
                redraw = 1;
            }
        }

        if(modo==1 && vezJ==-1){
            if(redraw){
                al_clear_to_color(al_map_rgb(166, 179, 176));
                al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,50,ALLEGRO_ALIGN_CENTER,"Ordem de Jogada");

                desenharBotao(medfont,screenX/4-200,200,screenX/4+200,600,400,al_map_rgb(145, 209, 227),"Aleatório");
                desenharBotao(medfont,screenX/4*3-200,200,screenX/4*3+200,390,295,blue,"Primeiro");
                desenharBotao(medfont,screenX/4*3-200,410,screenX/4*3+200,600,505,red,"Segundo");
                redraw=0;
                al_flip_display();
            }
            int validou = 0;
            if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
                if(inRange(evento.mouse.x,screenX/4,200) && inRange(evento.mouse.y,400,200)){ vezJ = randomBool(); validou = 1; }
                if(inRange(evento.mouse.x,screenX/4*3,200) && inRange(evento.mouse.y,295,95)){ vezJ = 0; validou = 1; }
                if(inRange(evento.mouse.x,screenX/4*3,200) && inRange(evento.mouse.y,505,95)){ vezJ = 1; validou = 1; }
                if(validou){
                    al_resume_timer(timer);
                    timerRedraw = 1;
                    redraw = 1;
                    minutos = 0;
                    segundos = 0;
                }
            }
        }
        if(modo==1 && vezJ!=-1){
            if(jogador!=vezJ){
                computerAction(fichas,tabuleiro,tPts,jogador,turno);
                turno++;
                jogador = !jogador;
                redraw = 1;
                al_rest(0.5);
            }
        }

        if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1 && !pause && (modo == 0 || jogador==vezJ)){
            gameAction(evento,fichas,tabuleiro,tPts,&jogador,&turno,&selecionado);
            redraw = 1;
        }

        if(pause){
            int time = al_get_timer_count(timer)*al_get_timer_speed(timer);
            al_stop_timer(timer);
            pauseScreen(bigfont,medfont,pvx,&rodando,time,tPts,jogador,fichas,tabuleiro,turno,modo,vezJ,queue,menu,exit);
            pause = 0;
            al_resume_timer(timer);
            redraw = 1;
        }

        if(timerRedraw && (modo == 0 || vezJ != -1)){
            desenharTimer(medfont,minutos,segundos);
            timerRedraw = 0;
            al_flip_display();
        }
        if(redraw){
            if(rodando)
                interfacePartida(partidabg,bigfont,medfont,minutos,segundos,tPts,jogador,fichas,tabuleiro,selecionado,cores,turno,vezJ,modo,blueChip,redChip);
                
            if(save){
                int time = al_get_timer_count(timer)*al_get_timer_speed(timer);
                salvarJogo(modo,vezJ,tabuleiro,fichas,jogador,time,turno);
                al_draw_text(medfont,al_map_rgb(0, 255, 26),screenX-20,20,ALLEGRO_ALIGN_RIGHT,"Jogo Salvo!");
            }
            al_flip_display();
            redraw=0;

            if(save){
                al_rest(1.0);
                save = 0;
            }
        }
    }
    al_destroy_bitmap(blueChip);
    al_destroy_bitmap(redChip);
    al_destroy_bitmap(partidabg);
}


void historicoScreen(ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,ALLEGRO_FONT *smallfont,ALLEGRO_EVENT_QUEUE *queue,ALLEGRO_COLOR blue,ALLEGRO_COLOR red,int *menu,int *historico, int *exit, int *mainRd){
    int rodando = 1,displayQtd,ini=0,active = 2,redraw = 1,menorTempo[3],maiorTempo[3];
    ALLEGRO_BITMAP *blueChip = al_load_bitmap("assets/blueChipSmall.png"),*redChip = al_load_bitmap("assets/redChipSmall.png");
    ALLEGRO_BITMAP *bg = al_load_bitmap("assets/menuBG.png"),*papiro = al_load_bitmap("assets/papiro.png");
    FILE *arq = fopen("historico.txt","r");
    int linhas = partidasArquivo(arq,active);
    int sizeArray = (linhas>0)?linhas:1;
    Partida ptdsTodas[sizeArray],ptdsPvP[sizeArray],ptdsPvC[sizeArray],*ptdsActive;
    int lnPvP=0,lnPvC=0;
    if(linhas>0){
        lnPvP = getPartidas(ptdsPvP,0,arq,&menorTempo[0],&maiorTempo[0]);
        rewind(arq);
        lnPvC = getPartidas(ptdsPvC,1,arq,&menorTempo[1],&maiorTempo[1]);
        rewind(arq);
        getPartidas(ptdsTodas,2,arq,&menorTempo[2],&maiorTempo[2]);
        fclose(arq);
    }
    while(rodando){
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue,&evento);
        eventExit(evento,&rodando,historico,&rodando,exit);

        if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
            if(inRange(evento.mouse.x,110,80) && inRange(evento.mouse.y,70,30)){
                *menu = 1;
                *historico = 0;
                rodando = 0;
                *mainRd = 1;
            }
        }
        
        displayQtd = 3;
        char linha[50];
        int i = 0,lns=0;

        if(linhas>0){
            if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
                int last = active;
                if(inRange(evento.mouse.x,screenX/2-150,50) && inRange(evento.mouse.y,95,25)) active = 2;
                if(inRange(evento.mouse.x,screenX/2,50) && inRange(evento.mouse.y,95,25)) active = 0;
                if(inRange(evento.mouse.x,screenX/2+150,50) && inRange(evento.mouse.y,95,25)) active = 1;
                if(active != last) ini = 0;
            }
            switch(active){
                case 0: ptdsActive = ptdsPvP; break;
                case 1: ptdsActive = ptdsPvC; break;
                case 2: ptdsActive = ptdsTodas; break;
            }
            int lnActive = (active==0)?lnPvP:(active==1)?lnPvC:linhas;
            if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
                if(lnActive>displayQtd){
                    if(ini>0 && inRange(evento.mouse.x,screenX/2-180,100) && inRange(evento.mouse.y,780,20)){
                        ini = ini-3;
                    }
                    if(ini+3<lnActive && inRange(evento.mouse.x,screenX/2+180,100) && inRange(evento.mouse.y,780,20)){
                        ini = ini+3;
                    }
                }
                redraw = 1;
            }

            if(displayQtd+ini>lnActive){displayQtd = lnActive-ini;}
            Partida ptds[3];
            for(int j = 0;j<displayQtd;j++)
                ptds[j] = ptdsActive[j+ini];


            if(redraw){
                al_clear_to_color(al_map_rgb(0,0,0));
                al_draw_bitmap(bg,0,0,0);

                desenharBotao(medfont,30,40,190,100,60,al_map_rgb(145,209,227),"Voltar");
                
                al_draw_bitmap(papiro,screenX/2-310,20,0);

                desenharBotao(smallfont,screenX/2-200,70,screenX/2-100,120,90,(active==2)?al_map_rgb(182, 145, 64):al_map_rgb(255, 228, 169),"Todos");
                desenharBotao(smallfont,screenX/2-50,70,screenX/2+50,120,90,(active==0)?al_map_rgb(182, 145, 64):al_map_rgb(255, 228, 169),"PvP");
                desenharBotao(smallfont,screenX/2+100,70,screenX/2+200,120,90,(active==1)?al_map_rgb(182, 145, 64):al_map_rgb(255, 228, 169),"PvC");

                char texto[20];
                sprintf(texto,"Menor Tempo - %02d:%02d",menorTempo[active]/60,menorTempo[active]%60);
                al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2,170,ALLEGRO_ALIGN_CENTER,texto);
                sprintf(texto,"Maior Tempo - %02d:%02d",maiorTempo[active]/60,maiorTempo[active]%60);
                al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2,200,ALLEGRO_ALIGN_CENTER,texto);
                
                for(int i=0;i<displayQtd;i++) displayPartida(230+i*150,ptds,i,smallfont,blue,red,blueChip,redChip);

                if(lnActive>displayQtd){
                    if(ini>0){
                        desenharBotao(smallfont,screenX/2-200,760,screenX/2-120,800,770,al_map_rgb(255, 228, 169),"<-");
                    }
                    if(ini+3<lnActive){
                        desenharBotao(smallfont,screenX/2+120,760,screenX/2+200,800,770,al_map_rgb(255, 228, 169),"->");
                    }
                    char texto[20];
                    sprintf(texto,"Página %d/%d",ini/3+1,(lnActive+2)/3); 
                    al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2,760,ALLEGRO_ALIGN_CENTER,texto);
                    sprintf(texto,"%d-%d de %d",ini+1,ini+displayQtd,lnActive); 
                    al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2,780,ALLEGRO_ALIGN_CENTER,texto);
                }
                redraw = 0;
                al_flip_display();
            }
        }else{
            if(redraw){
                desenharBotao(medfont,30,40,190,100,60,al_map_rgb(145,209,227),"Voltar");
                al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,400,ALLEGRO_ALIGN_CENTER,"Nenhuma partida no histórico.");
                redraw = 0;
                al_flip_display();
            }
        }
    }
    al_destroy_bitmap(blueChip);
    al_destroy_bitmap(redChip);
    al_destroy_bitmap(papiro);
    al_destroy_bitmap(bg);
}

void ajudaScreen(ALLEGRO_EVENT_QUEUE *queue,ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,ALLEGRO_FONT *smallfont,int *exit, int *help, int *menu){
    int pagina = 1,redraw = 1,rodando = 1;
    ALLEGRO_BITMAP *bg = al_load_bitmap("assets/menuBG.png"),*tab = al_load_bitmap("assets/screenshotVitoria.png");
    while(rodando){
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue,&evento);

        eventExit(evento,&rodando,help,help,exit);
        if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
            if(inRange(evento.mouse.x,130,110) && inRange(evento.mouse.y,810,30) && pagina>1) pagina--;
            if(inRange(evento.mouse.x,screenX-130,110) && inRange(evento.mouse.y,810,30) && pagina<3) pagina++;
            redraw = 1;
            if(inRange(evento.mouse.x,110,80) && inRange(evento.mouse.y,50,30)){
                rodando = 0;
                *menu = 1;
                *help = 0;
            }
        }

        if(redraw){
            al_draw_bitmap(bg,0,0,0);
            al_draw_filled_rounded_rectangle(30,100,screenX-30,screenY-100,10,10,al_map_rgb(215, 219, 218));
            al_draw_rounded_rectangle(30,100,screenX-30,screenY-100,10,10,al_map_rgb(0,0,0),4);

            desenharBotao(medfont,30,20,190,80,40,al_map_rgb(145,209,227),"Voltar");
            if(pagina>1) desenharBotao(medfont,20,780,240,840,800,al_map_rgb(145,209,227),"Anterior");
            if(pagina<3) desenharBotao(medfont,screenX-240,780,screenX-20,840,800,al_map_rgb(145,209,227),"Próximo");

            if(pagina==1){
                al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,130,ALLEGRO_ALIGN_CENTER,"Regras do Tri-Angle");
                al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2,180,ALLEGRO_ALIGN_CENTER,"O objetivo do jogo é alinhar 3 peças para vencer o oponente");
                al_draw_bitmap(tab,screenX/2-150,210,0);
                al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2,500,ALLEGRO_ALIGN_CENTER,"O jogo possui duas fases:");
                al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2-420,540,0,"1 - Fase de Posicionamento:");
                al_draw_multiline_text(smallfont,al_map_rgb(0,0,0),screenX/2-420,570,400,20,0,"Cada jogador posiciona uma peça por turno, clicando onde deseja posicionar.");
                al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2+50,540,0,"2 - Fase de Movimento:");
                al_draw_multiline_text(smallfont,al_map_rgb(0,0,0),screenX/2+50,570,400,20,0,"Cada jogador movimenta uma peça por turno, clicando para selecionar uma peça e clicando novamente na sua posição final desejada.");
                redraw = 0;
            }
            if(pagina==2){
                al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,130,ALLEGRO_ALIGN_CENTER,"Como jogar Tri-Angle");
                al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2,180,ALLEGRO_ALIGN_CENTER,"Como posicionar/movimentar as peças");
                al_draw_multiline_text(smallfont,al_map_rgb(0,0,0),screenX/2-440,220,450,20,0,"Na fase de posicionamento, basta clicar na casa em que deseja posicionar uma ficha e ela será posicionada imediatamente.");
                al_draw_multiline_text(smallfont,al_map_rgb(0,0,0),screenX/2+20,220,450,20,0,"Na fase de movimento, o jogador deve clicar em uma das suas peças posicionadas para selecioná-la e então clicar na casa em que deseja posicioná-la.\nObs: A peça selecionada será destacada em amarelo");
                al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2,420,ALLEGRO_ALIGN_CENTER,"Movimentos Permitidos");
                al_draw_multiline_text(smallfont,al_map_rgb(0,0,0),screenX/2-400,470,800,20,0,"No jogo TRI-Angle, as peças podem mover-se para um espaço vazio adjacente OU para um espaço vazio adjacente a outra peça em linha reta.\nDurante a fase de movimento, o jogo mostrará ao jogador quais movimentos são possíveis com cada peça através de uma linha da cor do jogador.");
                redraw = 0;
            }
            if(pagina==3){
                al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,130,ALLEGRO_ALIGN_CENTER,"Empate");
                al_draw_multiline_text(smallfont,al_map_rgb(0,0,0),screenX/2,190,700,20,ALLEGRO_ALIGN_CENTER,"Caso uma partida se prolongue, será considerada empatada ao chegar no turno 50.");
                al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,290,ALLEGRO_ALIGN_CENTER,"Recursos");
                al_draw_multiline_text(smallfont,al_map_rgb(0,0,0),screenX/2-230,350,470,20,ALLEGRO_ALIGN_CENTER,"SALVAR:\nPermite salvar a situação atual da partida para ser retomada em outro momento.");
                al_draw_multiline_text(smallfont,al_map_rgb(0,0,0),screenX/2+230,350,470,20,ALLEGRO_ALIGN_CENTER,"PAUSAR:\nPermite parar temporariamente o andamento da partida, sem atualizar o temporizador, para retomar quando desejado");
                redraw = 0;
            }
            al_flip_display();
        }
    }
    al_destroy_bitmap(bg);
    al_destroy_bitmap(tab);
}

int main(){
    ALLEGRO_DISPLAY *janela = NULL;
    int rodando=1,menu=1,help = 0,jogar = 0,historico = 0,pvp = 0,pvc = 0,exit = 0,mouse_x,mouse_y,redraw = 1;
    
    al_init();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_primitives_addon();
    al_init_image_addon();
    al_install_mouse();
    al_install_keyboard();

    ALLEGRO_FONT *bigfont = al_load_font("assets/PressStart2P.ttf",40,0);
    ALLEGRO_FONT *medfont = al_load_font("assets/PressStart2P.ttf",24,0);
    ALLEGRO_FONT *smallfont = al_load_font("assets/PressStart2P.ttf",16,0);

    ALLEGRO_COLOR blue = al_map_rgb(23, 203, 252);
    ALLEGRO_COLOR red = al_map_rgb(227, 48, 48);
    ALLEGRO_COLOR buttonColor = al_map_rgb(145, 209, 227);

    janela = al_create_display(screenX,screenY);

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue,al_get_mouse_event_source());
    al_register_event_source(queue,al_get_keyboard_event_source());
    al_register_event_source(queue,al_get_display_event_source(janela));
 
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);
    al_register_event_source(queue, al_get_timer_event_source(timer));

    Ponto tPts[7];
    tPts[0] = (Ponto){screenX/2,150,0,1};
    tPts[1] = (Ponto){screenX/2-135,(500*sqrt(3)/4)+150,1,0};
    tPts[2] = (Ponto){screenX/2,(500*sqrt(3)/4)+150,1,1};
    tPts[3] = (Ponto){screenX/2+135,(500*sqrt(3)/4)+150,1,2};
    tPts[4] = (Ponto){screenX/2-250,550,2,0};
    tPts[5] = (Ponto){screenX/2,550,2,1};
    tPts[6] = (Ponto){screenX/2+250,550,2,2};

    srand(time(NULL));

    while(rodando){
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue,&evento);
        
        if(jogar){
            selectGamemodeScreen(bigfont,medfont,smallfont,timer,tPts,&pvp,&pvc,&jogar,&menu,&exit,blue,red,buttonColor,queue);
        }
        if(historico){
            historicoScreen(bigfont,medfont,smallfont,queue,blue,red,&menu,&historico,&exit,&redraw);
        }
        if(help){
            al_stop_timer(timer);
            al_set_timer_count(timer,0);
            ajudaScreen(queue,bigfont,medfont,smallfont,&exit,&help,&menu);
        }
        if(menu){
            int yOff = 200;
            if(redraw){
                redraw = 0;
                menuScreen(bigfont,medfont,yOff);
            }
            if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button == 1){
                if(evento.mouse.x>=screenX/2-150 && evento.mouse.x<=screenX/2+150){
                    if(evento.mouse.y>=yOff+100 && evento.mouse.y<=yOff+160){
                        jogar = 1; menu = 0; redraw=1;
                    }
                    if(evento.mouse.y>=yOff+200 && evento.mouse.y<=yOff+260){
                        help = 1; menu = 0; redraw=1;
                    }
                    if(evento.mouse.y>=yOff+300 && evento.mouse.y<=yOff+360){
                        historico = 1; menu = 0;
                    }
                    if(evento.mouse.y>=yOff+400 && evento.mouse.y<=yOff+460){
                        exit = 1; menu = 0;
                    }
                }
            }
        }
        if(pvp||pvc){
            al_stop_timer(timer);
            al_set_timer_count(timer,0);
            int tabuleiro[3][3],fichas[2] = {3,3};
            for(int i=0;i<=2;i++){
                for(int j=0;j<=2;j++) tabuleiro[i][j]=-1;
            }
            if(pvp) jogarPartida(0,0,tabuleiro,fichas,0,0,1,bigfont,medfont,queue,blue,red,&exit,&pvp,&menu,timer,tPts,0);
            if(pvc) jogarPartida(1,-1,tabuleiro,fichas,0,0,1,bigfont,medfont,queue,blue,red,&exit,&pvc,&menu,timer,tPts,0);
        }
        
        if(exit)
            rodando = 0;

        if(evento.type==ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE || evento.type==ALLEGRO_EVENT_DISPLAY_CLOSE)
            rodando = 0;

        al_flip_display();
    }
    
    al_destroy_display(janela);
    al_destroy_font(bigfont);
    al_destroy_font(medfont);
    al_destroy_font(smallfont);
    al_destroy_event_queue(queue);

    return 0;
}