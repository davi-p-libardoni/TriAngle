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
    int matriz[3][3],minutos,segundos,vitoria,modo;
} Partida;
int inRange(int n,int target,int r){
    if(n>=target-r && n<=target+r) return 1;
    return 0;
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

int getPartidas(Partida ptds[],int modo, FILE *arq){
    int flag = 1,i=0;
    if(modo!=2) flag = 0;
    char linha[50];
    while(fgets(linha,50,arq)){
        int modoTmp;
        fgets(linha,sizeof(linha),arq);
        sscanf(linha,"modo %d",&modoTmp);
        if(modoTmp==modo || modo==2){
            ptds[i].modo = modoTmp;
            for(int j=0;j<3;j++){
                fgets(linha,sizeof(linha),arq);
                sscanf(linha,"%d %d %d",&ptds[i].matriz[j][0],&ptds[i].matriz[j][1],&ptds[i].matriz[j][2]);
            }
            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"minutos %d",&ptds[i].minutos);

            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"segundos %d",&ptds[i].segundos);

            fgets(linha,sizeof(linha),arq);
            sscanf(linha,"vitoria %d",&ptds[i].vitoria);

            fgets(linha,sizeof(linha),arq);
            i++;
        }else{
            for(int j=0;j<7;j++) fgets(linha,sizeof(linha),arq);
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
        if(strcmp(l,"<JOGO>")==0){
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

void irParaLinha(FILE *arq,int posicao){
    if(arq==NULL) return;
    char linha[50];
    for(int i=0;i<posicao;i++){
        fgets(linha,50,arq);
    }
}

void desenharFichas(int fichas[2],int tabuleiro[3][3],int jogador,Ponto *tPts,int selecionado,int size){
    int i,ini[2] = {100,screenX/2+50},fim[2] = {screenX/2-50,screenX-100},iniY=700,fimY=800,l,c;
    ALLEGRO_BITMAP *chips[2];
    int chipOffset;
    if(!size){
        chips[0] = al_load_bitmap("assets/blueChip.png");
        chips[1] = al_load_bitmap("assets/redChip.png");
        chipOffset = 40;
    }else{
        chips[0] = al_load_bitmap("assets/blueChipSmall.png");
        chips[1] = al_load_bitmap("assets/redChipSmall.png");
        chipOffset = 10;
    }
    
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

void displayPartida(int yOff,Partida ptds[3],int i,ALLEGRO_FONT *smallfont,ALLEGRO_COLOR red,ALLEGRO_COLOR blue){
    al_draw_filled_rounded_rectangle(screenX/2-250,yOff,screenX/2+250,yOff+150,8,8,al_map_rgb(128, 146, 142));

    int triMeio = screenX/2-140,triOff=60;
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
    desenharLinhaVitoria(pts,ptds[i].matriz,(ptds[i].vitoria)?red:blue,ptds[i].vitoria,5);
    desenharFichas(fichas,ptds[i].matriz,ptds[i].vitoria,pts,-1,1);

    char texto[20];
    sprintf(texto,"Vencedor: %s",(ptds[i].vitoria)?"Vermelho":"Azul");
    al_draw_text(smallfont,(ptds[i].vitoria)?red:blue,screenX/2-70,20+yOff,0,texto);
    sprintf(texto,"Tempo: %02d:%02d",ptds[i].minutos,ptds[i].segundos);
    al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2-70,20+40+yOff,0,texto);
    sprintf(texto,"Modo: %s",(ptds[i].modo==0)?"PvP":"PvC");
    al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2-70,20+80+yOff,0,texto);
}

void victoryScreen(int *pvp,int *pvprodando,int *menu,int *exit,int tabuleiro[3][3],int modo,ALLEGRO_TIMER *timer,int turnos,Ponto tPts[7],ALLEGRO_COLOR cor,int *fichas,int jogador,ALLEGRO_EVENT_QUEUE *queue,ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont){
    int rodando = 1,seconds = al_get_timer_count(timer)*al_get_timer_speed(timer),minutos = seconds/60,segundos = seconds%60;
    char text[30];

    FILE *arq = fopen("historico.txt","a");
    fprintf(arq,"<JOGO>\n");
    fprintf(arq,"modo %d\n",modo);
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

        sprintf(text,"Vitória do Jogador %d!",jogador+1);
        al_draw_text(bigfont,cor,screenX/2,50,ALLEGRO_ALIGN_CENTER,text);

        desenharTabuleiro(tPts,500);
        desenharLinhaVitoria(tPts,tabuleiro,cor,jogador,8);
        desenharFichas(fichas,tabuleiro,jogador,tPts,8,0);

        if(evento.type=ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
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
        
        al_draw_filled_rounded_rectangle(screenX/2-385,720,screenX/2-15,800,10,10,al_map_rgb(145, 209, 227));
        al_draw_rounded_rectangle(screenX/2-385,720,screenX/2-15,800,10,10,al_map_rgb(0,0,0),3);
        al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2-200,750,ALLEGRO_ALIGN_CENTER,"Jogar novamente");

        al_draw_filled_rounded_rectangle(screenX/2+15,720,screenX/2+385,800,10,10,al_map_rgb(145,209,227));
        al_draw_rounded_rectangle(screenX/2+15,720,screenX/2+385,800,10,10,al_map_rgb(0,0,0),3);
        al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2+200,750,ALLEGRO_ALIGN_CENTER,"Voltar ao menu");

        al_flip_display();
    }
}

void menuScreen(ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,int yOff){
    al_clear_to_color(al_map_rgb(166, 179, 176));
    Ponto smallPts[7];
    ALLEGRO_BITMAP *banner = al_load_bitmap("assets/banner.png"); 
    smallPts[0] = (Ponto){screenX/2,50};
    smallPts[1] = (Ponto){screenX/2-25,100};
    smallPts[2] = (Ponto){screenX/2,100};
    smallPts[3] = (Ponto){screenX/2+25,100};
    smallPts[4] = (Ponto){screenX/2-50,150};
    smallPts[5] = (Ponto){screenX/2,150};
    smallPts[6] = (Ponto){screenX/2+50,150};

    al_draw_bitmap(banner,screenX/8-118,0,0);
    al_draw_bitmap(banner,screenX/8*7-118,0,0);
    al_destroy_bitmap(banner);

    desenharTabuleiro(smallPts,120);
    
    al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,yOff,ALLEGRO_ALIGN_CENTER,"TRI-Angle");

    al_draw_filled_rounded_rectangle((screenX/2)-150,yOff+100,(screenX/2)+150,yOff+160,10,10,al_map_rgb(145, 209, 227));
    al_draw_rounded_rectangle((screenX/2)-150,yOff+100,(screenX/2)+150,yOff+160,10,10,al_map_rgb(0,0,0),4);
    al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2,yOff+120,ALLEGRO_ALIGN_CENTER,"Jogar");

    al_draw_filled_rounded_rectangle((screenX/2)-150,yOff+200,(screenX/2)+150,yOff+260,10,10,al_map_rgb(145, 209, 227));
    al_draw_rounded_rectangle((screenX/2)-150,yOff+200,(screenX/2)+150,yOff+260,10,10,al_map_rgb(0,0,0),4);
    al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2,yOff+220,ALLEGRO_ALIGN_CENTER,"Como jogar");

    al_draw_filled_rounded_rectangle((screenX/2)-150,yOff+300,(screenX/2)+150,yOff+360,10,10,al_map_rgb(145, 209, 227));
    al_draw_rounded_rectangle((screenX/2)-150,yOff+300,(screenX/2)+150,yOff+360,10,10,al_map_rgb(0,0,0),4);
    al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2,yOff+320,ALLEGRO_ALIGN_CENTER,"Histórico");

    al_draw_filled_rounded_rectangle((screenX/2)-150,yOff+400,(screenX/2)+150,yOff+460,10,10,al_map_rgb(145, 209, 227));
    al_draw_rounded_rectangle((screenX/2)-150,yOff+400,(screenX/2)+150,yOff+460,10,10,al_map_rgb(0,0,0),4);
    al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2,yOff+420,ALLEGRO_ALIGN_CENTER,"Sair");
}

void selectGamemodeScreen(ALLEGRO_FONT *bigfont,ALLEGRO_FONT *smallfont,ALLEGRO_COLOR blue, ALLEGRO_COLOR red){
    ALLEGRO_BITMAP *iconPvP = al_load_bitmap("assets/icon_pvp.png");
    ALLEGRO_BITMAP *iconPvC = al_load_bitmap("assets/icon_pc.png");
    al_clear_to_color(al_map_rgb(166, 179, 176));

    al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,50,ALLEGRO_ALIGN_CENTER,"Selecionar Modo de Jogo");

    al_draw_filled_rectangle((screenX/4)-200,150,(screenX/4),750,blue);
    al_draw_filled_rectangle((screenX/4),150,(screenX/4)+200,750,red);
    al_draw_rectangle((screenX/4)-200,150,(screenX/4)+200,750,al_map_rgb(0,0,0),4);
    al_draw_bitmap(iconPvP,screenX/4-100,265,0);
    al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/4,550,ALLEGRO_ALIGN_CENTER,"PvP");
    al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/4,590,ALLEGRO_ALIGN_CENTER,"Player vs Player");

    int offsetX = screenX/4*3;
    al_draw_filled_rectangle(offsetX-200,150,offsetX+200,750,blue);
    al_draw_rectangle(offsetX-200,150,offsetX+200,750,al_map_rgb(0,0,0),4);
    al_draw_bitmap(iconPvC,offsetX-100,265,0);
    al_draw_text(bigfont,al_map_rgb(0,0,0),offsetX,550,ALLEGRO_ALIGN_CENTER,"PvC");
    al_draw_text(smallfont,al_map_rgb(0,0,0),offsetX,590,ALLEGRO_ALIGN_CENTER,"Player vs. Computer");

    al_destroy_bitmap(iconPvP);
    al_destroy_bitmap(iconPvC);
}

void gamemodeButton(ALLEGRO_EVENT evento,int *jogar,int *pvp,int *pvc){
    int offPvP=screenX/4-200, offPvC=((screenX/4)*3)-200, offY=150, endY=750;
    if(evento.mouse.y>=offY && evento.mouse.y<=endY){
        if(evento.mouse.x>=offPvP && evento.mouse.x<=offPvP+400){
            *pvp = 1;
            *jogar = 0;
        }
        if(evento.mouse.x>=offPvC && evento.mouse.x<=offPvC+400){
            *pvc = 1;
            *jogar = 0;
        }
    }
}

void partidaPvP(ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,ALLEGRO_EVENT_QUEUE *queue,ALLEGRO_COLOR blue, ALLEGRO_COLOR red,ALLEGRO_DISPLAY *janela,int *exit,int *pvp,int *menu, ALLEGRO_TIMER *timer, Ponto tPts[7]){
    int fichas[2] = {3,3},turno = 1,jogador=0,rodando=1,tabuleiro[3][3],selecionado=-1,redraw=1;
    char text[50];
    ALLEGRO_COLOR cores[] = {blue,red};
    
    al_start_timer(timer);
    for(int i=0;i<=2;i++){
        for(int j=0;j<=2;j++) tabuleiro[i][j]=-1;
    }
    int segundosTotal,minutos,segundos;
    while(rodando){
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue,&evento);

        eventExit(evento,&rodando,pvp,pvp,exit);

        if (evento.type == ALLEGRO_EVENT_TIMER) {
            redraw = 1;
            segundosTotal = al_get_timer_count(timer) * al_get_timer_speed(timer);
            minutos = segundosTotal/60;
            segundos = segundosTotal%60;
        }

        int vitoria = checkVitoria(tabuleiro);
        if(vitoria!=-1){
            al_stop_timer(timer);
            victoryScreen(pvp,&rodando,menu,exit,tabuleiro,0,timer,turno,tPts,cores[vitoria],fichas,vitoria,queue,bigfont,medfont);
        }

        if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
            for(int i=0;i<7;i++){
                if(inRange(evento.mouse.x,tPts[i].x,20) && inRange(evento.mouse.y,tPts[i].y,20)){
                    if(fichas[jogador]>0 && tabuleiro[tPts[i].arrL][tPts[i].arrC] == -1){
                        fichas[jogador]--;
                        tabuleiro[tPts[i].arrL][tPts[i].arrC] = jogador;
                        jogador = !jogador;
                        turno++;
                    }
                    if(fichas[jogador]==0){
                        if(tabuleiro[tPts[i].arrL][tPts[i].arrC] == jogador){
                            selecionado = i;
                        }
                        if(selecionado!=-1){
                            if(isValidMove(tPts[selecionado].arrL,tPts[selecionado].arrC,tPts[i].arrL,tPts[i].arrC,tabuleiro)){
                                tabuleiro[tPts[selecionado].arrL][tPts[selecionado].arrC] = -1;
                                tabuleiro[tPts[i].arrL][tPts[i].arrC] = jogador;
                                jogador = !jogador;
                                turno++;
                                selecionado = -1;
                            }
                        }
                    }
                }
            }
            
        }
        
        if(redraw){
            al_clear_to_color(al_map_rgb(166, 179, 176));

            sprintf(text,"%02d:%02d",minutos,segundos);
            al_draw_text(medfont,al_map_rgb(0,0,0),50,25,0,text);

            desenharTabuleiro(tPts,500);
            
            if(fichas[jogador]==0) mostrarMovimentos(jogador,tabuleiro,tPts,selecionado,cores);

            desenharFichas(fichas,tabuleiro,jogador,tPts,selecionado,0);
            
            sprintf(text,"Turno %d",turno);
            al_draw_textf(bigfont,al_map_rgb(0,0,0),screenX/2,30,ALLEGRO_ALIGN_CENTER,text);

            int position = (jogador==0)?screenX/4:screenX/4*3;
            sprintf(text,"Vez do Jogador %d",jogador+1);
            if(rodando){
                al_draw_text(medfont,cores[jogador],position,70,ALLEGRO_ALIGN_CENTER,text);
            }
    
            redraw=0;
            al_flip_display();
        }
    }
}

int selecionarPrimeiro(){
    srand(time(NULL));
    return rand()%2;
}

void partidaPvC(ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,ALLEGRO_EVENT_QUEUE *queue,ALLEGRO_COLOR blue, ALLEGRO_COLOR red,ALLEGRO_DISPLAY *janela,int *exit,int *pvc,int *menu, ALLEGRO_TIMER *timer, Ponto tPts[7]){
    int primeiroJ,select=-1,redraw=1;
    if(select==-1){
        if(redraw==1){
            al_clear_to_color(al_map_rgb(166, 179, 176));
            al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,50,ALLEGRO_ALIGN_CENTER,"Ordem de Jogada");
            
            al_draw_filled_rounded_rectangle(screenX/4-200,200,screenX/4+200,600,10,10,al_map_rgb(145, 209, 227));
            al_draw_rounded_rectangle(screenX/4-200,200,screenX/4+200,600,10,10,al_map_rgb(0,0,0),3);
            al_draw_text(medfont,al_map_rgb(0,0,0),screenX/4,400,ALLEGRO_ALIGN_CENTER,"Aleatório");

            al_draw_filled_rounded_rectangle(screenX/4*3-200,200,screenX/4*3+200,390,10,10,blue);
            al_draw_rounded_rectangle(screenX/4*3-200,200,screenX/4*3+200,390,10,10,al_map_rgb(0,0,0),3);
            al_draw_text(medfont,al_map_rgb(0,0,0),screenX/4*3,295,ALLEGRO_ALIGN_CENTER,"Primeiro");

            al_draw_filled_rounded_rectangle(screenX/4*3-200,410,screenX/4*3+200,600,10,10,red);
            al_draw_rounded_rectangle(screenX/4*3-200,410,screenX/4*3+200,600,10,10,al_map_rgb(0,0,0),3);
            al_draw_text(medfont,al_map_rgb(0,0,0),screenX/4*3,505,ALLEGRO_ALIGN_CENTER,"Segundo");
            
            redraw=0;
            al_flip_display();
        }
    }else{
        primeiroJ = (select==2)?selecionarPrimeiro():select;
    }
}

void historicoScreen(ALLEGRO_FONT *bigfont,ALLEGRO_FONT *medfont,ALLEGRO_FONT *smallfont,ALLEGRO_EVENT_QUEUE *queue,ALLEGRO_COLOR blue,ALLEGRO_COLOR red,ALLEGRO_TIMER *timer,int *menu,int *historico, int *exit, int *mainRd){
    int rodando = 1,displayQtd,ini=0,active = 2,redraw = 1;
    al_start_timer(timer);
    FILE *arq = fopen("historico.txt","r");
    int linhas = partidasArquivo(arq,active);
    int sizeArray = (linhas>0)?linhas:1;
    Partida ptdsTodas[sizeArray],ptdsPvP[sizeArray],ptdsPvC[sizeArray],*ptdsActive;
    int lnPvP=0,lnPvC=0;
    if(linhas>0){
        lnPvP = getPartidas(ptdsPvP,0,arq);
        rewind(arq);
        lnPvC = getPartidas(ptdsPvC,1,arq);
        rewind(arq);
        getPartidas(ptdsTodas,2,arq);
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

        if(evento.type==ALLEGRO_EVENT_TIMER){
            redraw = 1;
        }

        if(linhas>0){
            switch(active){
                case 0: ptdsActive = ptdsPvP; break;
                case 1: ptdsActive = ptdsPvC; break;
                case 2: ptdsActive = ptdsTodas; break;
            }
            int lnActive = (active==0)?lnPvP:(active==1)?lnPvC:linhas;
            if(displayQtd+ini>lnActive){displayQtd = lnActive-ini;}
            Partida ptds[3];
            for(int j = 0;j<displayQtd;j++)
                ptds[j] = ptdsActive[j+ini];

            if(redraw){
                al_clear_to_color(al_map_rgb(166, 179, 176));
                al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,50,ALLEGRO_ALIGN_CENTER,"Histórico");

                al_draw_filled_rounded_rectangle(30,40,190,100,10,10,al_map_rgb(145, 209, 227));
                al_draw_rounded_rectangle(30,40,190,100,10,10,al_map_rgb(0,0,0),4);
                al_draw_text(medfont,al_map_rgb(0,0,0),110,60,ALLEGRO_ALIGN_CENTER,"Voltar");
                
                al_draw_filled_rounded_rectangle(screenX/2-300,90,screenX/2+300,800,10,10,al_map_rgb(215, 219, 218));
                al_draw_rounded_rectangle(screenX/2-300,90,screenX/2+300,800,10,10,al_map_rgb(0,0,0),3);

                al_draw_filled_rounded_rectangle((screenX/2)-275,100,(screenX/2)-125,160,10,10,(active==2)?al_map_rgb(71, 101, 94):al_map_rgb(145, 209, 227));
                al_draw_rounded_rectangle((screenX/2)-275,100,(screenX/2)-125,160,10,10,al_map_rgb(0,0,0),4);
                al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2-200,120,ALLEGRO_ALIGN_CENTER,"Todos");

                al_draw_filled_rounded_rectangle((screenX/2)-75,100,(screenX/2)+75,160,10,10,(active==0)?al_map_rgb(71, 101, 94):al_map_rgb(145, 209, 227));
                al_draw_rounded_rectangle((screenX/2)-75,100,(screenX/2)+75,160,10,10,al_map_rgb(0,0,0),4);
                al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2,120,ALLEGRO_ALIGN_CENTER,"PvP");

                al_draw_filled_rounded_rectangle((screenX/2)+125,100,(screenX/2)+275,160,10,10,(active==1)?al_map_rgb(71, 101, 94):al_map_rgb(145, 209, 227));
                al_draw_rounded_rectangle((screenX/2)+125,100,(screenX/2)+275,160,10,10,al_map_rgb(0,0,0),4);
                al_draw_text(medfont,al_map_rgb(0,0,0),screenX/2+200,120,ALLEGRO_ALIGN_CENTER,"PvC");
                
                for(int i=0;i<displayQtd;i++){
                    int yOff = 170+i*190;
                    displayPartida(180+i*190,ptds,i,smallfont,red,blue);
                }

                if(lnActive>displayQtd){
                    if(ini>0){
                        al_draw_filled_rounded_rectangle((screenX/2)-250,740,(screenX/2)-100,780,10,10,al_map_rgb(145, 209, 227));   
                        al_draw_rounded_rectangle((screenX/2)-250,740,(screenX/2)-100,780,10,10,al_map_rgb(0,0,0),4);
                        al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2-175,750,ALLEGRO_ALIGN_CENTER,"Anterior");
                    }
                    if(ini+3<lnActive){
                        al_draw_filled_rounded_rectangle((screenX/2)+100,740,(screenX/2)+250,780,10,10,al_map_rgb(145, 209, 227));   
                        al_draw_rounded_rectangle((screenX/2)+100,740,(screenX/2)+250,780,10,10,al_map_rgb(0,0,0),4);
                        al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2+175,750,ALLEGRO_ALIGN_CENTER,"Próximo");
                    }
                    char texto[20];
                    sprintf(texto,"Página %d/%d",ini/3+1,(lnActive+2)/3); 
                    al_draw_text(smallfont,al_map_rgb(0,0,0),screenX/2,750,ALLEGRO_ALIGN_CENTER,texto);
                }
                redraw = 0;
                al_flip_display();
            }

            if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
                int last = active;
                if(inRange(evento.mouse.x,screenX/2-200,75) && inRange(evento.mouse.y,130,30)) active = 2;
                if(inRange(evento.mouse.x,screenX/2,75) && inRange(evento.mouse.y,130,30)) active = 0;
                if(inRange(evento.mouse.x,screenX/2+200,75) && inRange(evento.mouse.y,130,30)) active = 1;
                if(active != last) ini = 0;
            }

            if(lnActive>displayQtd){
                if(ini>0){
                    if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button == 1 && inRange(evento.mouse.x,screenX/2-180,100) && inRange(evento.mouse.y,750,30)){
                        ini = ini-3;
                    }
                }
                if(ini+3<lnActive){
                    if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button == 1 && inRange(evento.mouse.x,screenX/2+180,100) && inRange(evento.mouse.y,750,30)){
                        ini = ini+3;
                    }
                }
            }
        }else{
            if(redraw){
                al_draw_text(bigfont,al_map_rgb(0,0,0),screenX/2,400,ALLEGRO_ALIGN_CENTER,"Nenhuma partida no histórico.");
                redraw = 0;
                al_flip_display();
            }
        }
    }
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

    ALLEGRO_FONT *bigfont = al_load_font("PressStart2P-Regular.ttf",32,0);
    ALLEGRO_FONT *medfont = al_load_font("PressStart2P-Regular.ttf",24,0);
    ALLEGRO_FONT *smallfont = al_load_font("PressStart2P-Regular.ttf",16,0);

    ALLEGRO_COLOR blue = al_map_rgb(23, 203, 252);
    ALLEGRO_COLOR red = al_map_rgb(227, 48, 48);

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

    while(rodando){
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue,&evento);
        
        if(jogar){
            if(redraw){
                redraw = 0;
                selectGamemodeScreen(bigfont,smallfont,blue,red);
            }
            if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP && evento.mouse.button==1){
                gamemodeButton(evento,&jogar,&pvp,&pvc);
                redraw=1;
            }
        }

        if(historico){
            al_stop_timer(timer);
            al_set_timer_count(timer,0);
            historicoScreen(bigfont,medfont,smallfont,queue,blue,red,timer,&menu,&historico,&exit,&redraw);
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
            
        if(pvp){
            al_stop_timer(timer);
            al_set_timer_count(timer,0);
            partidaPvP(bigfont,medfont,queue,blue,red,janela,&exit,&pvp,&menu,timer,tPts);
        }
        
        if(pvc){
            al_stop_timer(timer);
            al_set_timer_count(timer,0);
            partidaPvC(bigfont,medfont,queue,blue,red,janela,&exit,&pvc,&menu,timer,tPts);
        }
        
        if(exit)
            rodando = 0;

        if(evento.type==ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE || evento.type==ALLEGRO_EVENT_DISPLAY_CLOSE)
            rodando = 0;

        al_flip_display();
    }
    
    al_destroy_display(janela);
    al_destroy_event_queue(queue);

    return 0;
}