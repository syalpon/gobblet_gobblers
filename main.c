#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*�}�N����`*/
#define ON  1
#define OFF 0
#define DEBUG_STEP_FALG OFF
#define DEBUG_SHOW_FLAG OFF
#define HEEP_EXPAND(Z)    do{\
    if( Z > 1000000 )\
    {\
        printf("!!! allocation 1000000 byte !!!\n");\
        exit(1);\
    }\
    Z += sizeof(Moves);\
}while(0)
#define HEEP_REDUCE(Z)    do{Z -= sizeof(Moves);}while(0)
#define SHOW_USE_HEEP(Z)  do{printf("use_heep_size = %u [datanum = %d]\n",Z,Z/sizeof(Moves));}while(0)



/*-----�^��`-----*/
typedef enum _Size
{
  NONE     =0,
  SMALL    =1,
  MIDDLE   =2,
  LARGE    =3
}Size;

typedef enum _Player
{
  RandomPlayer =0,
  Player1      =1,
  Player2      =2
}Player;

typedef enum _POINT
{
    OUTSIDE = 0,
    INDEX1  = 1,
    INDEX2  = 2,
    INDEX3  = 3
}Point;

typedef struct _Vector
{
    Point x;
    Point y;
}Vector;

typedef struct _Koma
{
    const Player  owner;
    const Size    size;
    Vector        place;
    struct _Koma *inside;
    struct _Koma *outside;
}Koma;

typedef struct _Moves
{
    struct _Moves *next;
    Vector   point;
    Koma    *koma;
    Player  winner;
}Moves;

typedef struct _STATUS
{
    struct _STATUS *next;
    unsigned int a;
    unsigned int b;
    unsigned char c;
}STATUS;


/*-----�O���[�o���ϐ�-----*/
static unsigned int use_heep_size = 0;
Koma dummy = {RandomPlayer,NONE,{0,0},NULL,NULL};
STATUS *status = NULL;
Koma komas[2][6] = 
{
    {
        {Player1,SMALL ,{OUTSIDE,OUTSIDE},NULL,NULL},
        {Player1,SMALL ,{OUTSIDE,OUTSIDE},NULL,NULL},
        {Player1,MIDDLE,{OUTSIDE,OUTSIDE},NULL,NULL},
        {Player1,MIDDLE,{OUTSIDE,OUTSIDE},NULL,NULL},
        {Player1,LARGE ,{OUTSIDE,OUTSIDE},NULL,NULL},
        {Player1,LARGE ,{OUTSIDE,OUTSIDE},NULL,NULL}
    },
    {
        {Player2,SMALL ,{OUTSIDE,OUTSIDE},NULL,NULL},
        {Player2,SMALL ,{OUTSIDE,OUTSIDE},NULL,NULL},
        {Player2,MIDDLE,{OUTSIDE,OUTSIDE},NULL,NULL},
        {Player2,MIDDLE,{OUTSIDE,OUTSIDE},NULL,NULL},
        {Player2,LARGE ,{OUTSIDE,OUTSIDE},NULL,NULL},
        {Player2,LARGE ,{OUTSIDE,OUTSIDE},NULL,NULL}
    }
};

Koma *Board[3][3]=
{
    {&dummy,&dummy,&dummy},
    {&dummy,&dummy,&dummy},
    {&dummy,&dummy,&dummy}
};




/*----- �v���g�^�C�v�錾 -----*/
void    show_board(void);
void    show_hand(void);
int     move(Point ,Point ,Koma *);
int     dry_move(Point ,Point ,Koma *,Player *);
Player  judge(void);  
Moves  *get_valid_moves(Player);
Moves  *get_best_moves(Player);
void    show_moves_list(Moves *);
void    show_a_move(Moves *);
void    clear_moves_list(Moves **);
void    register_move(Moves **,Point ,Point ,Koma *);
Moves*  exsist_win_move(Moves *,Player);
int     is_visit(Player);
int     stack_heepdata(Moves *);
Moves  *pop_heepdata();




/*----- ���C���֐� -----*/
int main(void)
{
    Moves *list;
//    move(INDEX2,INDEX2,&komas[0][0]);
//    move(INDEX1,INDEX2,&komas[0][3]);
//    move(INDEX2,INDEX2,&komas[0][3]);
//    move(INDEX1,INDEX2,&komas[0][3]);
//    move(INDEX1,INDEX1,&komas[0][3]);
//    move(INDEX3,INDEX1,&komas[1][2]);
//    move(INDEX3,INDEX2,&komas[1][3]);
    list = get_best_moves(Player1);
    printf("------------\n");
    //printf("ret = %d\n",(int)dry_move(INDEX3,INDEX3,&komas[0][4],NULL));
    SHOW_USE_HEEP( use_heep_size );
    clear_moves_list(&list);
    SHOW_USE_HEEP( use_heep_size );
 
    return 0;
}

/* �Ֆʕ\�� */
void show_board(void)
{
    int i,j;

    show_hand();

    printf("---------\n");
    for(j=0;j<3;j++)
    {
        for(i=0;i<3;i++)
        {
            Koma *p = Board[j][i];
            char *str = " ";

            if( p != NULL )
            {
                if( p->owner == Player1 )
                {
                    str = "o";
                }
                if( p->owner == Player2 )
                {
                    str = "x";
                }
            }

            printf("%s",str);
            
            if( p != NULL )
            {
                if( p->size == SMALL )
                {
                    str = "1";
                }
                if( p->size == MIDDLE )
                {
                    str = "2";
                }
                if( p->size == LARGE )
                {
                    str = "3";
                }
            }
            printf("%s|",str);
        }
        printf("\n");
    }
    printf("---------\n");
}

/* ������\�� */
void show_hand(void)
{
    int i,j;

    for(i=0;i<2;i++)
    {
        for(j=0;j<6;j++)
        {
            Koma *p = &komas[i][j];
            if( p->place.x==0 || p->place.y==0 )
            {
                printf("%s%d ",p->owner==Player1?"o":(p->owner==Player2?"x":""),p->size);
            }
            else
            {
                printf("   ");
            }
        }
        printf("\n");
    }
}

/* ��u�� */
/* �Ԓl�F1=�u����, 0=�u���Ȃ����� */
int move(Point x,Point y,Koma *koma)
{
    Koma *old_koma = (x>0 && y>0) ? Board[y-1][x-1] : NULL;
    int ret = 0;

    /* NULL�`�F�b�N */
    if( koma == NULL )
    {
        return ret;
    }

    #if DEBUG_SHOW_FLAG
        printf("---------------------------------\n");
        show_board();
        printf("koma [%s%d] ",koma->owner==Player1?"o":(koma->owner==Player2?"x":""),koma->size);
        printf("(%d,%d) -> (%d,%d)\n",koma->place.x,koma->place.y,x,y);
    #endif

    /*�O���ɋ����̂œ������Ȃ��ꍇ*/
    if( koma->outside != NULL )
    {
        /* Do Nothing */
    }
    /*���ɖ߂��ꍇ*/
    else if( old_koma == NULL )
    {
        Board[koma->place.y-1][koma->place.x-1] = 
        (koma->inside == NULL) ? &dummy : (koma->inside->outside = NULL,koma->inside);

        koma->place.x     = x;
        koma->place.y     = y;
        koma->inside      = NULL;
        koma->outside     = NULL;
    }
    /*�u����ꍇ*/
    else if( old_koma->size < koma->size )
    {
        Point before_x = koma->place.x;
        Point before_y = koma->place.y;

        /*�ړ��������ɔՏ�ɂ���ꍇ*/
        if( before_x>0 && before_y>0 )
        {
            Board[before_y-1][before_x-1] = (koma->inside == NULL) ? &dummy : koma->inside;
        }

        Board[y-1][x-1]   = koma;
        koma->place.x     = x;
        koma->place.y     = y;

        if( old_koma != &dummy )
        {
            old_koma->outside = koma;    
        }

        koma->inside      = old_koma;
        
        ret = 1;
    }

    #if DEBUG_SHOW_FLAG 
        printf(">>>%s\n",ret==0?"failed":"success");
        show_board();
        #if DEBUG_STEP_FALG
        getchar();
        #endif
    #endif

    return ret;
}

/* ���u�� */
/* �Ԓl�F1=�u����, 0=�u���Ȃ����� */
/* winner:0=�ǂ���������Ȃ� 1=Player1������ 2=Player2������ */
int dry_move(Point x,Point y,Koma *koma,Player *winner)
{
    Point   old_x = koma->place.x;
    Point   old_y = koma->place.y;
    Point   result;
    int     ret;

    ret = move(x,y,koma);

    if( ret == 1 )
    {
        result = judge();

        if( winner != NULL )
        {
            *winner = result;
        }

        move(old_x,old_y,koma);
    }

    return ret;
}

/* ���s���� */
/* �Ԓl�F0=�ǂ���������Ȃ� 1=Player1������ 2=Player2������ */
Player judge(void)
{
    int x,y;
    
    #define LINES(x1,y1,x2,y2,x3,y3,z) \
    if( Board[x1-1][y1-1]->owner==z && \
        Board[x2-1][y2-1]->owner==z && \
        Board[x3-1][y3-1]->owner==z )\
    {\
        return z;\
    }

    /* y�s�� */
    for(y=1;y<=3;y++)
    {
        LINES(y,INDEX1,y,INDEX2,y,INDEX3,Player1);
        LINES(y,INDEX1,y,INDEX2,y,INDEX3,Player2);
    }

    /* x��� */
    for(x=1;x<=3;x++)
    {
        LINES(INDEX1,x,INDEX2,x,INDEX3,x,Player1);
        LINES(INDEX1,x,INDEX2,x,INDEX3,x,Player2);
    }

    /*�΂� \ */
    LINES(INDEX1,INDEX1,INDEX2,INDEX2,INDEX3,INDEX3,Player1);
    LINES(INDEX1,INDEX1,INDEX2,INDEX2,INDEX3,INDEX3,Player2);

    /*�΂� / */
    LINES(INDEX1,INDEX3,INDEX2,INDEX2,INDEX3,INDEX1,Player1);
    LINES(INDEX1,INDEX3,INDEX2,INDEX2,INDEX3,INDEX1,Player2);
    #undef LINES

    return RandomPlayer;
}

/* �Ԓl�F�w�����̃��X�g(���ȎQ�ƃ|�C���^) */
Moves *get_valid_moves(Player player)
{
    int i,j,x,y;
    Moves *list = NULL;

    /*player�̋�S�Ă�����*/
    for(j=0;j<2;j++)
    {
        for(i=0;i<6;i++)
        {
            if( komas[j][i].owner == player )
            {
                /*�Ֆʏ�̃}�X������*/
                for(y=1;y<=3;y++)
                {
                    for(x=1;x<=3;x++)
                    {
                        if( dry_move(x,y,&komas[j][i],NULL) )
                        {
                            register_move(&list,x,y,&komas[j][i]);
                        }
                    }
                }
            }
        }
    }
    return list;
}

/* ���X�g�ւ̓o�^ */
void register_move(Moves **p,Point x,Point y,Koma *koma)
{
    Moves *newdata;

    if( p != NULL)
    {
        Player player;

        HEEP_EXPAND(use_heep_size);
        newdata = malloc(sizeof(Moves));
        newdata->point.x = x;
        newdata->point.y = y;
        newdata->koma = koma;
        dry_move(x,y,koma,&player);
        newdata->winner = player;
        newdata->next = *p;

        *p = newdata;
    }
}

/* ���X�g�̑S�\�� */
void show_moves_list(Moves *list)
{
    int num=0;
    while( list != NULL )
    {
        Koma *p = list->koma;

        num++;
        printf("%d:[%s%d] ",num,p->owner==1?"o":"x",p->size);
        printf("(%d %d)->(%d,%d) : win=%d\n",p->place.x,p->place.y,list->point.x,list->point.y,list->winner);
        list = list->next;
    }
}

/* ���X�g�擪�̕\�� */
void show_a_move(Moves *list)
{
    if( list != NULL )
    {
        Koma *p = list->koma;
        printf("[%s%d] ",p->owner==1?"o":"x",p->size);
        printf("(%d %d)->(%d,%d) : win=%d\n",p->place.x,p->place.y,list->point.x,list->point.y,list->winner);
    }
    else
    {
#if DEBUG_SHOW_FLAG
    printf("error : NULL pointer access in show_a_move()\n");
#endif
    }
}

/* ���X�g�̃�������� */
void clear_moves_list(Moves **list)
{
    if( list != NULL )
    {
        Moves *p;
        while( *list != NULL)
        {
            p = *list;
            *list = (*list)->next;
            HEEP_REDUCE(use_heep_size);
            free(p);
        }
    }
}

/* �Ԓl:���Ƃ��˂��̎� , ������肵���Ȃ�or�u���Ȃ���NULL */
Moves *get_best_moves(Player player)
{
    Moves *valid_moves_list;
    Moves *win_move;
    Moves *ret;

#if DEBUG_SHOW_FLAG
    printf(" [ called function ] %p\n",get_best_moves);
#endif
    /* �w������S�Ď擾 */
    valid_moves_list = get_valid_moves(player);
#if DEBUG_SHOW_FLAG
    show_moves_list(valid_moves_list);
#endif

    /* �w����肪�����Ȃ��ꍇ��NULL */
    if( valid_moves_list == NULL )
    {
        return NULL;
    }

    /* ���Ă��̒T�� */
    win_move = exsist_win_move(valid_moves_list,player);

    /* �ǂ̎���w���Ă����ĂȂ�(=������肵���Ȃ�)�ꍇ */
    if( (win_move == NULL) && (exsist_win_move(valid_moves_list,RandomPlayer)==NULL) )
    {
        return NULL;
    }

#if DEBUG_SHOW_FLAG
    printf("<win move> (%p)\n",win_move);
    show_a_move(win_move);
#endif

    /* ����1���player�̏��肪�Ȃ��ꍇ */
    if( win_move == NULL )
    {
        Moves *p = valid_moves_list ;

        while( p != NULL )
        {
            if(p->winner == 0)
            {
                /* ��̒���1�u������� */
                move(p->point.x,p->point.y,p->koma);

                /* ���ɓ����ՖʂŌ������Ă���ӏ��ł���Ό����ȗ� */
                if( is_visit((player&1)+1) == 1 )
                {
                    break;
                }

                /* HEEP���������ꎞ�I�ɉ�� */
                stack_heepdata(p);

                /* ����̍őP���T�� */
                if( get_best_moves( (player&1)+1 ) != NULL )
                {
                    /* ����̍őP�肪���遁������ */
                    p->winner = (player&1)+1;
                }
                else
                {
                    /* ����̍őP�肪�Ȃ������Ă� */
                    p->winner = player;
                }

                /* ���1�߂� */
                move(p->koma->place.x,p->koma->place.y,p->koma);

                /* �ꎞ�I�ɉ������HEEP���������Ď擾 */
                pop_heepdata(p);

                /* ���Ă����������Ȃ猟���X�g�b�v */
                if( p->winner == player )
                {
                    break;
                }
            }
            /* ���̎������ */
            p = p->next;
        }

        /* deep�T����(0�ɂȂ��Ă���winner�͂Ȃ�)������x���� */
        win_move = exsist_win_move(valid_moves_list,player);
    }
    
    /* ����������O�Ƀf�[�^�R�s�[ */
    ret = malloc(sizeof(Moves));
    HEEP_EXPAND(use_heep_size);
    ret->next    = NULL;
    ret->point.x = win_move->point.x;
    ret->point.y = win_move->point.y;
    ret->koma    = win_move->koma;
    ret->winner  = win_move->winner;
    
    /* �������N���A */
    printf("����������O : ");
    SHOW_USE_HEEP( use_heep_size );
    clear_moves_list(&valid_moves_list);
    printf("����������� : ");
    SHOW_USE_HEEP( use_heep_size );
    return ret;
}

/* ���� */
/* �Ԓl�F�����؂�����=��, �Ȃ�=NULL */
Moves* exsist_win_move(Moves *list,Player player)
{
    while( list != NULL )
    {
        if(list->winner == player)
        {
            return list;
        }
        list = list->next;
    }

    return NULL;
}

/* ����̔Ֆʂ����ɋN�������̊m�F */
/* �Ԓl�F1=�K�ꂽ,0=�����B */
int is_visit(Player player)
{
    /* �ŏ�ʌ� : ���w��player(1,2) */
    /* koma 1...12 �̏ꏊ [0-9]*12 */
    int i,j;
    FILE *fp = NULL;
    char line[16] = "";
    char target[16] = "";
    char *cp;
#define FILE_NAME "database.txt"

    /* ���񌟍�����ID */
    target[0] = '0'+player;
    for(i=0;i<2;i++)
    {
        for(j=0;j<6;j++)
        {
            Koma *p = &komas[i][j];
            if( (p->place.x==0) || (p->place.y==0) )
            {
                target[6*i+j+1] = '0';
            }
            else
            {
                target[6*i+j+1] = '0'+(p->place.y-1)*3+(p->place.x-1);
            }
        }
    }

    /* ���� */
    if (!(fp = fopen(FILE_NAME,"r")))
    {
        /* �t�@�C�����Ȃ��ꍇ�V�K�쐬 */
        fp = fopen(FILE_NAME,"w");
        fclose(fp);
    }
    else
    {
        while ( cp=fgets(line, sizeof(line), fp) ) 
        {
            if( strstr(line,target) != NULL )
            {
                break;
            }
        }
        fclose(fp);
    }
    
    /* cp��NULL�̎��͌�����Ȃ������Ƃ� */
    if( cp == NULL )
    {
        fp = fopen(FILE_NAME,"a");
        fprintf(fp,"%s",target);
        fprintf(fp,"\n");
        fclose(fp);
    }

    
#if DEBUG_STEP_FLAG
    getchar();
#endif
   
#undef FILE_NAME
}



int stack_heepdata(Moves *list)
{
    int ret = 0;
    char *cp;
    char line[64] = "";
    FILE *fp;

#define STACK_HEEP_DATA_FILE_NAME "stack_heep_data.txt"

    /* ���� */
    if (!(fp = fopen(STACK_HEEP_DATA_FILE_NAME,"r")))
    {
        /* �t�@�C�����Ȃ��ꍇ�V�K�쐬 */
        fp = fopen(STACK_HEEP_DATA_FILE_NAME,"w");
        fclose(fp);
    }
    else
    {
        while ( cp=fgets(line, sizeof(line), fp) ) 
        {
            if( strstr(line,target) != NULL )
            {
                break;
            }
        }
        fclose(fp);
    }   
    return ret;
#undef STACK_HEEP_DATA_FILE_NAME
}


Moves *pop_heepdata()
{
    return NULL;
}






