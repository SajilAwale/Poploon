#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <conio.h>
#include <dos.h>
#include <time.h>
#include <math.h>
#define TITLE "Pooploon"
#define SCREEN_WIDTH 800                //1360
#define SCREEN_HEIGHT 600               //700
#define TARGET_HEIGHT 500
#define xcen -79  // x cordinate center of ellipse
#define sr 65    // radius of smaller balloon
#define br 85   // radius of bigger radius
#define NO_BU 1000   /*currently this is the variable that controls how many bubble will be produced*/
#define H_S_LOCATION "HighScore.dat"
#define B_DEL 400
#define INF_DEL 300
#define TOP_SC 10
#define FRAME_DEL 50  //about 20 fps
#define CY_RANGE 5
#define CX_RANGE 2
#define RNX_BUBB 5
typedef struct screen
{
    int c:12;       // c for counters i.e i,j,k,l,m
    int cx:8;           //for uncertinity of x
    int cy:8;          // for uncertinity of y
    unsigned int curr_y:12;
    int curr_x:11;
    unsigned int asize:8;   //for storing the size (radius)of bubble in the screen
    unsigned int color:4;       // 0 to 2^4-1  = 15
}s;
typedef struct score_data
{
    char name[51];      //not sure how to input string in graphics mode
    unsigned int score;
    char date_t[31];
}h_s_d;
typedef struct addtional_info
{
    int indi;     // indicates how many bullunes are on memory/have been created
    int saver1;     // for CPU efficency
    int ssh;        // ssh --> stop scoring high
    unsigned int prev_click:1;   //for ckhech_w_b function logic
}inf;

// prototype area
void over(int score,char *reason);
int score_keeper(char change);
int check_w_b(s **sp, inf *inf1);
int rnd(short int ll,short int hl);
int rd();
int step_process();
int bu_gen(s **sp,inf *inf1);
void target_box();
s *allocate_the_memory();
int welcom();
int c_fptr(FILE *fptr);
int check_h_m_h_s_i_f();


short int ycen = SCREEN_HEIGHT/2;
short int oTARGET_WIDTH  = SCREEN_WIDTH/3;

void want_ur_name(h_s_d *h)
{
    closegraph();
    puts("Enter your name for High Score\n");
    fflush(stdin);
    scanf("%s",(h->name));
    puts("\nThanks, You can press Enter to return to Gaming");
    fflush(stdin);
    getchar();
    initwindow(SCREEN_WIDTH,SCREEN_HEIGHT,TITLE,true,true);
}
int UP_SCORE(s **sp, int t_b)
{
    int d;  //distance
    int score;
    d = sqrt(pow((sp[t_b]->curr_x +sp[t_b]->c) - (mousex()),2) + pow((sp[t_b]->curr_y) - (mousey()),2));
    if(d == 0)  return ((sp[t_b]->asize));
    score = (sp[t_b]->asize)/(d);
    return score;
}
int D_h_s_sort()       //returns 0 if sucessfull
{
    int struct_size, i, j;
    h_s_d h1, h2;
    struct_size = sizeof(h1);
    FILE * fptr = fopen(H_S_LOCATION,"rb+");
    if(c_fptr(fptr))   return 1;
    fseek(fptr, 0, SEEK_END);       //go to end of file
    int file_size = ftell(fptr);
    rewind(fptr);                   //go to begining of file
    for (i = 0; i < file_size - struct_size; i += struct_size)
    {
        for (j = 0; j < file_size - struct_size - i; j += struct_size)
        {
            fread(&h1, struct_size, 1, fptr);           //rem that poniter itself moved by 1 high score data so no need to move it manually
            fread(&h2, struct_size, 1, fptr);
             if (h1.score < h2.score)
            {
                fseek(fptr, -(struct_size * 2), SEEK_CUR);
                fwrite(&h2, struct_size, 1, fptr);
                fwrite(&h1, struct_size, 1, fptr);
                fseek(fptr, -struct_size, SEEK_CUR);
            }
            else
            {
                fseek(fptr, -struct_size, SEEK_CUR);
            }
        }
        rewind(fptr);
    }
    fclose(fptr);
    return 0;
}

int save_h_s_iff(int score)       //save high score if and only if complex condition satisfy
{
    h_s_d h;
    time_t the_time = time(NULL);
    FILE *fptr = fopen(H_S_LOCATION,"rb+");
    if(c_fptr(fptr))   return 0;
    h.score = score;
    strcpy(h.date_t,ctime(&the_time));
    if(check_h_m_h_s_i_f() < TOP_SC)
    {
        want_ur_name(&h);
        fseek(fptr,0,SEEK_END); //to continue writing
        if(fwrite(&h,sizeof(h),1,fptr) < 1)
        {
        perror("Could not save high score " H_S_LOCATION);
        fclose(fptr);
        return 0;
        }
        fclose(fptr);
        return 1;       //this function returns 1 if successfull
    }
    else
    {
        h_s_d h1;
        want_ur_name(&h);
        fseek(fptr,-sizeof(h1),SEEK_END);
        fwrite(&h, sizeof(h1), 1, fptr);
        fclose(fptr);
        return 1;
    }
    fclose(fptr);
    return 0;
}

int is_it_h_s(int score)
{
    FILE *fptr = fopen(H_S_LOCATION,"rb");
    h_s_d h1;
    if(c_fptr(fptr))   return 0;
    if(check_h_m_h_s_i_f() < TOP_SC)
    {
        fclose(fptr);
        return 1;
    }
    D_h_s_sort();
    fseek(fptr,-sizeof(h1),SEEK_END);
    fread(&h1,sizeof(h1),1,fptr);
    if(h1.score < score)
    {
        fclose(fptr);
        return 1;
    }
    fclose(fptr);
    return 0;
}
void free_rem_sp(s **sp, int saver1,int iPlus1)
{
    int i;
    for(i=saver1;i<iPlus1;i++)
    {
        free(sp[i]);
    }
    free(sp);
}
int c_fptr(FILE *fptr)
{
    if(fptr == NULL)
    {
        perror("Coulnot Open file" H_S_LOCATION);
        return 1;
    }
    return 0;
}
void b_but(short int x, short int y, char str[], char color)
{
    setcolor(color);
    outtextxy(x,y,str);
}
int check_h_m_h_s_i_f()     //check how many high score data is in the file + it creates the file H_S_LOCATION.dat if absent
{
    int i;
    h_s_d temp;
    FILE *fptr = fopen(H_S_LOCATION,"ab");      // create the file H_S_Location.dat if absent
        if(c_fptr(fptr))   exit(1);     //checks fptr
    fclose(fptr);
    fptr = fopen(H_S_LOCATION,"rb");
    if(c_fptr(fptr))   exit(1);
    for(i=0;fread(&temp,sizeof(temp),1,fptr) == 1;i++);     //second opton of this code is to use fseek and ftell divid by sizeof temp    fclose(fptr);
    fclose(fptr);
    return i;
}
void over(int score,char reason[])
{
    char g_over[] = "GAME OVER !!!";
    char sc[15];
    int bkcolor = 0; //black
    int localbol = is_it_h_s(score);
    if(localbol)    bkcolor = 4; //red
    setbkcolor(bkcolor);
    settextstyle(3,0,8);
    outtextxy((getmaxx()/2)-(textwidth(g_over)/2),(getmaxy()/2)-(textheight(g_over)/2),g_over);
    sprintf(sc,"Score is %d",score);
    settextstyle(3,0,2);
    outtextxy((getmaxx()/2)-(textwidth(reason)/2),(getmaxy()/2)-3*(textheight(reason)),reason);
    settextstyle(3,0,5);
    outtextxy((getmaxx()/2)-(textwidth(sc)/2),(getmaxy()/2)+(textheight(sc)/1.5),sc);
    delay(3000);
    if(localbol)
        save_h_s_iff(score);
    return;
}
int score_keeper(char change)
{
    static int sc_n=0;       //score in numbers
    char score[101];
    sc_n += change;
    sprintf(score,"Score: %d",sc_n);
    settextstyle(3,0,3);
    b_but(getmaxx()-textwidth(score),textheight(score),score,BLACK);
    setcolor(WHITE);
    return sc_n;
}
int check_w_b(s **sp, inf *inf1)
{
    short int i;
    static char prev_i;
    for(i=(inf1->saver1);i<(inf1->indi);i++)
    {
        short int t = sp[i]->asize;      //max distance
        short int d = sqrt(pow((sp[i]->curr_x +sp[i]->c) - (mousex()),2) + pow((sp[i]->curr_y) - (mousey()),2));    //distance between pointer and bubble center
            if(d<=t && d>=(-t))
            {
                if(prev_i != i)
                    inf1->ssh = 0;      //reset the stop score high(ssh) if new bullune is on the target
                if(sp[i]->color != 15)
                {
                    return (prev_i=i);
                }

                if(sp[i]->color == 15 && inf1->prev_click == 1)
                    return (prev_i=i);
            }
        }

    return -1;          // i.e no bullunes lie at that position
}
unsigned long ud_rnd()
{
    static unsigned long a=126873519, b=398209817, c=578298231;
    unsigned long t;
    a ^= a << 16;
    a ^= a >> 5;
    a ^= a << 1;
    t = a;
    a = b;
    b = c;
    c = t ^ a ^ b;
    return c;
}
int rnd(short int ll,short int hl) // for generating random numbers from lower limit ll, to higher limit hl
{
    unsigned int raw_t = (time(NULL) + abs(ud_rnd()/100));
    srand(raw_t);
    return (rand()%(hl-ll+1) + ll);
}
int rd()
{
    if(rnd(0,1))    return br;
    else            return sr;
}
int step_process()
{
    int late, score = score_keeper(0);
    late = (12*score)/(sr) + 7;
    if(score >100 && score < 150)    late = rnd(25,35);
    if(score > 149)     late = rnd(30,50);
    return late;
}
void y_uncer_val(s **sp,short int i)
{
    short int dy = rnd(-CY_RANGE,CY_RANGE); //randomize y cordinates;
    if((sp[i]->curr_y + dy) > ((SCREEN_HEIGHT+TARGET_HEIGHT)/2 - sp[i]->asize))         // over do cases
        dy = rnd(-CY_RANGE,0);
    if((sp[i]->curr_y + dy) < ((SCREEN_HEIGHT-TARGET_HEIGHT)/2 + sp[i]->asize))         // overdo cases
        dy = rnd(0,CY_RANGE);
    sp[i]->cy = dy;
}
void x_uncer_val(s **sp,short int i)
{
    short int dx;
    if(i==0)
     return;
    dx = rnd(-CX_RANGE,CX_RANGE);
    if(((sp[i-1]->curr_x+sp[i-1]->c)-(sp[i]->curr_x +sp[i]->c+ dx))   <   (sp[i]->asize + sp[i-1]->asize))  dx = rnd(-CX_RANGE,0);
    if(((sp[i]->curr_x +sp[i]->c+ dx)-(sp[i+1]->curr_x+sp[i+1]->c))   <   (sp[i]->asize + sp[i+1]->asize))  dx = rnd(0,CX_RANGE);   //over do cases
    sp[i]->cx = dx;
}
int bu_gen(s **sp,inf *inf1)
{
    short int i;
    char b_t_r, t_b;        // is boolen variable for knowing if the mouse is witnin the target box and t_b is that i value
    static char page=0;
    setactivepage(page);
    setvisualpage(1-page);
    cleardevice();
    setlinestyle(0,0,2);
    if(!(GetAsyncKeyState(VK_LBUTTON) || GetAsyncKeyState(VK_RBUTTON)))
        inf1->prev_click = 0;
    for(i=(inf1->saver1);i<(inf1->indi);i++)
    {
        setcolor(WHITE);
        setfillstyle(1,sp[i]->color);
        sp[i]->curr_y += (sp[i]->cy);           //set current y position
        sp[i]->curr_x += (sp[i]->cx);           //set current x position
        fillellipse(sp[i]->curr_x +sp[i]->c,sp[i]->curr_y,sp[i]->asize,sp[i]->asize);// bubbles
        y_uncer_val(sp,i);  x_uncer_val(sp,i);    //randomize x any y cordinates
        setcolor(sp[i]->color);
    }
    setfillstyle(1,WHITE);  setcolor(RED);
    //fillellipse(mousex(),mousey(),1,1);                             // mouse center pointer
    b_t_r = ((mousex())>=((SCREEN_WIDTH-oTARGET_WIDTH)/2) && (mousex())<=((SCREEN_WIDTH+oTARGET_WIDTH)/2) && (mousey())>=(ycen-TARGET_HEIGHT/2) && (mousey())<=(ycen+TARGET_HEIGHT/2));
    t_b = check_w_b(sp,inf1); //check_which_bubble lies in the target area and store it's i value(identifier) of bubble to t_b
    if((GetAsyncKeyState(VK_LBUTTON) || GetAsyncKeyState(VK_RBUTTON)) && t_b == -1 && inf1->prev_click == 0)
    {
            setvisualpage(page);
            cleardevice();
            over(score_keeper(0),"No Balloon there");
            return 0;
    }
    if((GetAsyncKeyState(VK_LBUTTON) || GetAsyncKeyState(VK_RBUTTON))&&((inf1->ssh)++) == 0)
    {
        if(GetAsyncKeyState(VK_LBUTTON)&& sp[t_b]->asize == sr && b_t_r && inf1->prev_click==0)
        {
            setcolor(sp[t_b]->color = 15);      //disappear the bubble(white)
            score_keeper(UP_SCORE(sp,t_b));
            inf1->prev_click = 1;
        }
        else if(GetAsyncKeyState(VK_RBUTTON) && sp[t_b]->asize == br && b_t_r && inf1->prev_click==0)
        {
            setcolor(sp[t_b]->color = 15);      //disappear the bubble
            score_keeper(UP_SCORE(sp,t_b));
            inf1->prev_click = 1;
        }
        else if(!b_t_r)
        {
            setvisualpage(page);
            cleardevice();
            over(score_keeper(0),"Your cursor is outside target box");
            return 0;
        }

        else if(inf1->prev_click==0)
            {
                setvisualpage(page);
                cleardevice();
                over(score_keeper(0),"Wrong Click for ballon");
                return 0;
            }
    }
    page = 1-page;
    delay(FRAME_DEL);
    return 1;
}

void target_box()
{
    short int score = score_keeper(0);
    if(oTARGET_WIDTH > 199)  oTARGET_WIDTH = SCREEN_WIDTH/3 - score;
    if(oTARGET_WIDTH < 200)  oTARGET_WIDTH = rnd(150,200);
    setcolor(RED);
    setlinestyle(0,0,3);
    rectangle((SCREEN_WIDTH-oTARGET_WIDTH)/2, ycen-TARGET_HEIGHT/2,(SCREEN_WIDTH+oTARGET_WIDTH)/2,ycen+TARGET_HEIGHT/2);
    setlinestyle(1,0,1);
    line(SCREEN_WIDTH/2,ycen-TARGET_HEIGHT/2,SCREEN_WIDTH/2,ycen-TARGET_HEIGHT/2+TARGET_HEIGHT/16);      //vertical line obserable
    line(SCREEN_WIDTH/2,ycen+TARGET_HEIGHT/2,SCREEN_WIDTH/2,ycen+TARGET_HEIGHT/2-TARGET_HEIGHT/16);      //vertical line obserable
    line(SCREEN_WIDTH/2-oTARGET_WIDTH/2,ycen,SCREEN_WIDTH/2-oTARGET_WIDTH/2+oTARGET_WIDTH/16,ycen);      //horizontal line
    line(SCREEN_WIDTH/2+oTARGET_WIDTH/2,ycen,SCREEN_WIDTH/2+oTARGET_WIDTH/2-oTARGET_WIDTH/16,ycen);      //horizontal line
}

s *allocate_the_memory()
{
    s *ptr = (s *)malloc(sizeof(s));
    if(ptr == NULL)
    {
        puts("Malloc failed");
        delay(1000);
        exit(1);
    }
    return ptr;
}
void new_game()
{
    initwindow(SCREEN_WIDTH,SCREEN_HEIGHT,TITLE,true,true);
    s *sp[NO_BU+1];       // array to store address  ,+1 is not for terminating/ null char but is for error free in comming lines
    inf inf1;
    inf1.ssh = inf1.saver1 = inf1.prev_click= 0;
    int i,GAP;
    setlinestyle(SOLID_LINE,0,1);
    setbkcolor(WHITE);
    for(i=0;i<NO_BU;i++)
    {
        if(i==0)
        {
            sp[i] = allocate_the_memory();
            sp[i]->asize=rd();
            sp[i]->curr_x=xcen;
            sp[i]->c=1;
        }
        sp[i+1] = allocate_the_memory();
        for(sp[i+1]->c=1,sp[i+1]->asize=rd(),sp[i]->color=rnd(0,14),inf1.indi=i+1,sp[i]->cy = 0,sp[i]->cx = 0,sp[i]->curr_y=ycen,sp[i+1]->curr_x=xcen;;)     //i is the identifier of each bubblne here
        {
            int j;
            //runs only 1 time for one bullune
            if(sp[i]->c==1)
            {
                if(sp[i]->asize == sp[i+1]->asize)   // if consecutive bubble are of same size
                {
                    if(sp[i]->asize == sr)
                        GAP = rnd(2*sr, 3*sr);       // max gap between consecutive small bubble is 1.5 times min GAP
                    else                            //if(s1.asize[i] == br)
                        GAP = rnd(2*br, 3*br);       // max gap between consecutive big bubble is 1.5 times min GAP
                }
                else
                {
                    GAP = rnd(sr+br, 2*sr+br);      // max limit is approx 1.5 times min vlaue for current value
                }
            }

            if((sp[i]->c)>(GAP))      //c for counter
                break;
            if(bu_gen(sp, &inf1) != 1)
            {
                free_rem_sp(sp,inf1.saver1,i+1);
                score_keeper(-score_keeper(0));         //reset the score to 0 for new game
                oTARGET_WIDTH = SCREEN_WIDTH/3;
                return;
            }
            target_box();
            score_keeper(0);            // display current score
            for(j=(inf1.saver1);j<=i;j++)           //all the necessery increments for making all booluns moving
            {
                (sp[j]->c)+= step_process();
            }
            if((xcen+sp[inf1.saver1]->c) > getmaxx()+br)
            {
                free(sp[(inf1.saver1)]);
                inf1.saver1++;
            }
        }
    }
   getch();
   closegraph();
}
int ycalc(char nt,char n,char font=8, char txt_size=5)
{
    int total_heit,y,padding=10,txt_heit;
    settextstyle(font,0,txt_size);
    txt_heit = textheight("A");
    total_heit = nt*txt_heit + (nt-1)*padding;
    y = (SCREEN_HEIGHT/2 - total_heit/2) +(n-1)*(txt_heit + padding);
    return y;
}
int button(short int y, char *but_nam)
{
    int txt_color , b_bool; //black by default
    settextstyle(8,0,5);
    b_bool = mousex() > (SCREEN_WIDTH/2 - textwidth(but_nam)/2) && mousex() < (SCREEN_WIDTH/2 + textwidth(but_nam)/2) && (mousey()) > (y) && (mousey()) < (y+textheight(but_nam));
    txt_color = b_bool ? 2:0;   //green
    if(b_bool && GetAsyncKeyState(VK_LBUTTON))
    {
        b_but(SCREEN_WIDTH/2 - textwidth(but_nam)/2, y, but_nam, 11);
        delay(B_DEL);
        if(!GetAsyncKeyState(VK_LBUTTON))       //extra protection
        return 1;
    }
    b_but(SCREEN_WIDTH/2 - textwidth(but_nam)/2, y, but_nam, txt_color);
    return 0;
}
int welcom()
{
    initwindow(SCREEN_WIDTH,SCREEN_HEIGHT,TITLE,true,true);
    char title[] = TITLE;
    char mini_game[] = "Rxn Time";
    char we[][21] = {"Sajil Awale","Shrey Niraula","Pratik Luitel","Pujan Budhathoki"};
    int sep;
    int i, sum=0;
    int nt = 4;     //no of buttons
        cleardevice();
        // names
        {
            setbkcolor(WHITE);
            settextstyle(1,0,1);
            sep = (SCREEN_WIDTH - textwidth(we[0]) -textwidth(we[1]) -textwidth(we[2]) -textwidth(we[3]))/5;
            for(i=0;i<4;i++)
            {
                b_but((i+1)*sep+sum,SCREEN_HEIGHT-2*textheight(we[0]),we[i],i+2);
                sum += textwidth(we[i]);
            }
        }
        floodfill(SCREEN_WIDTH-1,SCREEN_HEIGHT-1,WHITE);    // for white background
    // for title
        {
            setcolor(5);
            settextstyle(3,0,7);
            b_but(SCREEN_WIDTH/2-textwidth(title)/2,SCREEN_HEIGHT/10,title,5);
        }
        //line(SCREEN_WIDTH/2-textwidth(ne_gam)/2,ycalc(nt,1),SCREEN_WIDTH/2+textwidth(ne_gam)/2,ycalc(nt,1));
        //line(SCREEN_WIDTH/2-textwidth(high_sc)/2,ycalc(nt,4)+textheight(high_sc),SCREEN_WIDTH/2+textwidth(high_sc)/2,ycalc(nt,4)+textheight(high_sc));
    for(char ne_gam[] = "New Game",high_sc[] = "High Score",close[] = "Exit",inst[] = "Instructions";;)
    {
        //buttons
        if(button(ycalc(nt,1),ne_gam))      return 0; //New game button
        if(button(ycalc(nt,4),high_sc))     return 1; //high score button
        if(button(SCREEN_HEIGHT-2*textheight(close),close))     return -1; // exit button
        if(button(ycalc(nt,2),mini_game))     return 2; //game
        if(button(ycalc(nt,3),inst))     return 3;
        delay(INF_DEL);
    }
}
int print_h_s()
{
    initwindow(SCREEN_WIDTH,SCREEN_HEIGHT,TITLE,true,true);
    FILE *fptr;
    h_s_d h1;
    int i;
    char h_s_s[]= "High Scores";
    char g_b_w_s[] = "Go Back";
    fptr = fopen(H_S_LOCATION,"rb");
    cleardevice();
    setcolor(5);
    floodfill(SCREEN_WIDTH-1,SCREEN_HEIGHT-1,WHITE);    // for white background
    setbkcolor(WHITE);
    if(c_fptr(fptr))    return 0;
    if(check_h_m_h_s_i_f() == 0)
    {
        char nofile[] = "Looks like it's you first time";
        b_but(SCREEN_WIDTH/2-textwidth(nofile)/2,SCREEN_HEIGHT/2-textheight(nofile)/2,nofile,0);
    }
    // title i.e High score
    if(D_h_s_sort())    puts("High score couldnot be sort");              //sort highscore in descending order
    settextstyle(8,0,5);
    b_but(SCREEN_WIDTH/2-textwidth(h_s_s)/2,textheight(h_s_s)/2,h_s_s,0);
    settextstyle(8,0,3);
    for(i=0;fread(&h1,sizeof(h1),1,fptr) == 1; i++)     //fread returns 1 if they were sucessful
    {
        char buffer[300];
        sprintf(buffer,"#%2d: %-7s : %d :On %s",i+1,h1.name,h1.score,h1.date_t);
        b_but(SCREEN_WIDTH/2-textwidth(buffer)/2,2*textheight(h_s_s)+(i+1)*textheight(buffer),buffer,0);
    }
    fclose(fptr);
    for(;;)
    {
        if(button(SCREEN_HEIGHT-1*textheight(g_b_w_s),g_b_w_s))   return 1;  //go back to welcome screen
        delay(INF_DEL);
    }

    return 0;
}
void rxn_game()
{
    char mess[21] = "OUT OUT !!!";
    char ins[] = "Click bullble as soon as bubble gets filled";
    int sum= 0,fill_color;
    initwindow(SCREEN_WIDTH,SCREEN_HEIGHT,TITLE,true,true);
    floodfill(SCREEN_WIDTH-1,SCREEN_HEIGHT-1,WHITE);
    setbkcolor(WHITE);  settextstyle(8,0,3);
    b_but(SCREEN_WIDTH/2-textwidth(ins)/2,SCREEN_HEIGHT/5,ins,0);
    for(int j=0;j<RNX_BUBB;j++)
    {
        int i;
        setcolor(1);    setfillstyle(1,15);
        fillellipse(SCREEN_WIDTH/2,SCREEN_HEIGHT/2,br,br);
        fill_color = rnd(0,14);
        while((GetAsyncKeyState(VK_LBUTTON)))   delay(20);
        delay(rnd(1000,5000));
        if((GetAsyncKeyState(VK_LBUTTON)))
        {
            //out out
            setbkcolor(15);
            settextstyle(3,0,5);
            b_but(SCREEN_WIDTH/2 - textwidth(mess)/2,SCREEN_HEIGHT/2 - textheight(mess)/2, mess, 0);
            delay(3000);
            return;
        }
            for(i=0;;i++)
            {
                setcolor(fill_color);
                setfillstyle(1,fill_color);       //COLOR(R,G,B)
                fillellipse(SCREEN_WIDTH/2,SCREEN_HEIGHT/2,br,br);// bubbles
                if(GetAsyncKeyState(VK_LBUTTON))
                {
                    if((sqrt(pow((SCREEN_WIDTH/2) - (mousex()),2) + pow((SCREEN_HEIGHT/2) - (mousey()),2))) < br)
                            break;
                }
                delay(1);
            }
            sum += i;
    }
    sprintf(mess,"Rxn time %.2f ms",(float)sum/RNX_BUBB);
    setbkcolor(fill_color);
    settextstyle(8,0,5);
    b_but(SCREEN_WIDTH/2 - textwidth(mess)/2,SCREEN_HEIGHT/2 - textheight(mess)/2, mess, 15);
    delay(3000);
    return;
}
int main()
{
    int gd = DETECT, gm;
    check_h_m_h_s_i_f();        // to create file if not created
    for(;;)
    {
        int r = welcom();
        closegraph();
        if(r == 0)
        {
            new_game();
            closegraph();
        }
        else if(r == 1)
        {
            if(print_h_s()==1)
            {
                closegraph();
                continue;
            }
            closegraph();
        }
        else if(r == 2)
        {
            rxn_game();
            closegraph();
        }
        else if(r == -1)
            break;
    }
    closegraph();
    return 0;
}


