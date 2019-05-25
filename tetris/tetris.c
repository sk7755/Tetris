#include "tetris.h"

static struct sigaction act, oact;

int main(){
	int exit=0;
	initscr();
	noecho();
	keypad(stdscr, TRUE);	

	srand((unsigned int)time(NULL));

	createRankList();

	recRoot = (RecNode*)malloc(sizeof(RecNode));
	recRoot->score = 0;
	recRoot->lv = -1;
	recRoot->f = (char (*) [WIDTH])malloc(sizeof(char)*HEIGHT*WIDTH);
	ConstructTree(recRoot);

	while(!exit){
		clear();
		switch(menu()){
		case MENU_PLAY: play(); break;
		case MENU_RANK: rank(); break;
		case MENU_EXIT: exit=1; break;
		case MENU_RECOMMEND: recommendedPlay(); break;
		default: break;
		}
	}

	endwin();
	system("clear");
	return 0;
}

void InitTetris(){
	int i,j;

	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			field[j][i]=0;
	nextBlock[0]=rand()%7;
	nextBlock[1]=rand()%7;
	nextBlock[2]=rand()%7;
	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;	
	gameOver=0;
	timed_out=0;

	DrawOutline();
	DrawField();
	DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void DrawOutline(){	
	int i,j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0,0,HEIGHT,WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2,WIDTH+10);
	printw("NEXT BLOCK");
	DrawBox(3,WIDTH+10,4,8);
	DrawBox(9,WIDTH+10,4,8);
	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(15,WIDTH+10);
	printw("SCORE");
	DrawBox(16,WIDTH+10,1,8);
}

int GetCommand(){
	int command;
	command = wgetch(stdscr);
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	default:
		break;
	}
	if(drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	return ret;	
}

void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
	for(i = 0 ; i < HEIGHT;i++)
	{
		for(j = 0 ; j < WIDTH;j++)
			recRoot->f[i][j] = field[i][j];
	}
	max = -100000;
	modified_recommend(recRoot);
	DrawRecommend(recommendY, recommendX, nextBlock[0], recommendR);
	
}


void PrintScore(int score){
	move(17,WIDTH+11);
	printw("%8d",score);
	move(HEIGHT,WIDTH+10);
}

void DrawNextBlock(int *nextBlock){
	int i, j;
	for( i = 0; i < 4; i++ ){
		move(4+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[1]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
	for(i=0;i<4;i++){
		move(10+i,WIDTH+13);
		for( j = 0; j<4; j++){
			if(block[nextBlock[2]][0][i][j] == 1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
				
	}
	move(HEIGHT,WIDTH+10);
}

void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y >=0 ){
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}
	move(HEIGHT,WIDTH+10);
}

void DrawBox(int y,int x, int height, int width){
	int i,j;
	move(y,x);
	addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for(j=0;j<height;j++){
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	move(y+j+1,x);
	addch(ACS_LLCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}
		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();
			newRank(score);
			writeRankFile();

			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
	writeRankFile();
}

char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	int i,j;

	for(i=0; i<BLOCK_HEIGHT; i++)
		for(j=0 ;j<BLOCK_WIDTH; j++)
		{
			if(block[currentBlock][blockRotate][i][j] && blockY+i >=0)
			{
				if(  (j+blockX < 0) || (WIDTH-1 <j+blockX) || (i+blockY > HEIGHT-1))
					return 0;

				if( f[i+blockY][j+blockX])
					return 0;
			}
		}	

	return 1;
}

void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX){
		int i,j;
		int tmpY, tmpX, tmpB, tmpS;
		int shadowY;

		tmpY = blockY;
		tmpX = blockX;
		tmpB = blockRotate;

		switch(command){
			case KEY_UP :   if(blockRotate -1 < 0)
						tmpB = 3;
				       	else
						tmpB = blockRotate-1;
					break;
			case KEY_DOWN : tmpY=blockY-1;
					break;
			case KEY_RIGHT : tmpX  = blockX-1;
					break;
			case KEY_LEFT : tmpX = blockX+1;
		}	
		shadowY = tmpY;

		while(CheckToMove(field, currentBlock,tmpB,shadowY+1, tmpX))
			shadowY++;

		for(i=0; i <BLOCK_HEIGHT;i++)
			for(j=0;j<BLOCK_WIDTH;j++)
				if(block[currentBlock][tmpB][i][j] == 1 && i+shadowY >= 0)
				{	
					move(shadowY+1+i,tmpX+1+j);
					printw(".");
				}


		for(i=0; i<BLOCK_HEIGHT; i++)
			for(j=0; j<BLOCK_WIDTH; j++)
				if(block[currentBlock][tmpB][i][j] == 1 && i+tmpY >= 0)
				{
					move(tmpY+1+i, tmpX+1+j);
					printw(".");
				}
		DrawRecommend(recommendY,recommendX,currentBlock,recommendR);
		DrawBlockWithFeatures(blockY,blockX,currentBlock,blockRotate);

		return;

	}

void BlockDown(int sig){
	int i,j;
	timed_out=0;
	if(CheckToMove( field, nextBlock[0], blockRotate, blockY+1, blockX))
	{
		blockY++;
		DrawChange(field,KEY_DOWN, nextBlock[0],blockRotate,blockY,blockX);
		return;
	}

	AddBlockToField(field, nextBlock[0], blockRotate,blockY, blockX);

	score += DeleteLine(field);
	PrintScore(score);
	nextBlock[0] = nextBlock[1];
	nextBlock[1] = nextBlock[2];
	nextBlock[2] = rand() % 7;
	
	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	DrawField(nextBlock[0], blockRotate, blockY, blockX);
	if(CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX))
	{
		DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
		DrawNextBlock(nextBlock);
	}
	else
		gameOver= TRUE;
	return ;
}

void AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	int i,j;
	for(i=0 ; i<BLOCK_HEIGHT; i++)
	{
		for(j=0;j<BLOCK_WIDTH; j++)
		{
			if(block[currentBlock][blockRotate][i][j] == 1)
			{
				if(f[i+blockY+1][j+blockX] == 1 || i+blockY+1 >= HEIGHT )
					score+= 10;
				f[i+blockY][j+blockX] = 1;
				if(i+blockY == 0)
					gameOver=TRUE;
			}
		}
	}

	return ;
}

int DeleteLine(char f[HEIGHT][WIDTH]){
	int i,j,k;
	int tmp;
	int DltLine=0;

	for(i=0; i<HEIGHT; i++)
	{
		tmp = 0;
		for(j =0 ; j < WIDTH; j++)
			if(f[i][j] == 1)
				tmp += 1;
		if (tmp == WIDTH) //i row is detected
		{
			DltLine++;
			for(k=i; k>0 ;k--)
				for(j=0;j<WIDTH; j++)
				{
					move(k+1,j+1); //k행 j열에 옮기고
					if(f[k][j]=f[k-1][j])
					{
						attron(A_REVERSE);
						printw(" ");
						attroff(A_REVERSE);
					}
					else
						printw(".");
				}
		}
	}
	return DltLine*DltLine* 100;
}

void DrawShadow(int y, int x, int blockID,int blockRotate){
	int i,j;
	
	while(CheckToMove(field,nextBlock[0],blockRotate,y+1,blockX))
		y++;

	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				printw("/");
			}
		}
	move(HEIGHT, WIDTH+10);

}

void DrawBlockWithFeatures(int y,int x, int blockID, int blockRotate)
{
	DrawShadow(y,x,blockID,blockRotate);
	DrawBlock(y,x,blockID,blockRotate,' ');

	return;

}

void createRankList(){
	FILE* fp;
	int n;
	int i,j;
	Node* pNew;
	Node* pCur;

	head = (Head*)malloc(sizeof(Head));
	head->link = NULL;
	head->num = 0;

	if(!(fp = fopen("rank.txt","r")))
	{
		printw("FILE OPEN ERROR!\n");
		getch();
		return;
	}

	fscanf(fp,"%d", &n);

	if( n == 0 )
	{
		//ERROR
	}
	for(i = 0 ; i < n ; i++)
	{
		pNew = (Node*)malloc(sizeof(Node));
		pNew-> link = NULL;

		fscanf(fp,"%s %d", pNew->name, &(pNew->score));

		if(head->num == 0)
		{
			head->link = pNew;
			pCur = head->link;
		}
		else
		{
			pCur->link = pNew;
			pCur = pCur->link;
		}
		(head->num)++;
			
	}
	fclose(fp);
	// user code
}

void rank(){
	int option;
	int x=1,y=head->num;
	int i,j;
	int r=0;
	int name_flag = 0; //if there's corresponding name flag = 1
	char name[NAMELEN]="";
	Node* pCur = head->link;
	Node* pDlt;

	clear();
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");
	option = wgetch(stdscr);
	echo();
	switch(option){
		case '1': printw("X: ");
			scanw("%d", &x);
			printw("Y: ");
			scanw("%d", &y);
			break;
		case '2': printw("input the name: ");
			scanw("%s", name);
			break;
		case '3': printw("input the rank: ");
			scanw("%d", &r);
			break;
		default :noecho(); 
			return;
	}
	noecho();

	if(option == '1'){
		printw("\tname\t\t|    score\n");
		printw("--------------------------------------\n");

		if(x > y || x > head->num || y < 1)
		{
			printw("\nsearch failure: no rank in the list");
			getch();
			return;
		}

		if(x < 1)
			x = 1;
		if (y > head->num)
			y = head->num;
	
		for(i=1; i< x; i++)
			pCur = pCur->link;
		j=0;
		for(i = x; i<=y;i++)
		{
			move(7+j,1);
			printw("%s",pCur->name);
			move(7+j,24);
			printw("| %d\n",pCur->score);
			j++;

			pCur = pCur->link ;
		}
	}
	else if(option == '2')
	{
		printw("\tname\t\t|    score\n");
		printw("--------------------------------------\n");

		pCur = head->link;
		name_flag = 0;
		j = 0;
		for(i=0;i<head->num;i++)
		{
			if( strcmp(pCur->name,name) == 0 )
			{
				name_flag = 1;
				move(6+j,1);
				printw("%s",pCur->name);
				move(6+j,24);
				printw("| %d\n",pCur->score);
				j++;
			}
			pCur = pCur->link;
		}
		if(!name_flag)
		{
			printw("\nsearch failure: no name in the list\n");
			getch();
			return;
		}
	}
	else if(option == '3')
	{
		pCur = head->link;
		if(!(1 <= r&& r <= head->num)) //print Error even if head->num = 0
		{
			printw("\nsearch failure: the rank not the list\n");
			getch();
			return;
		}

		if(r == 1)
		{
			pDlt = pCur;
			head->link = pCur->link;
		}
		else
		{
			for(i=2;i<r;i++)
				pCur = pCur->link;//pCur is pre element from element to be deleted
			pDlt = pCur->link;
			pCur->link = pDlt->link;
		}
		free(pDlt);
		head->num--;
		printw("\nresult: the rank deleted\n");
	}

	getch();
}

void writeRankFile(){
	Node* pCur = head->link;
	int i;
	FILE* fp;

	fp = fopen("rank.txt","w");

	fprintf(fp,"%d",head->num);
	
	for(i=0;i<head->num;i++)
	{
		fprintf(fp,"\n%s %d",pCur->name, pCur->score);
		pCur = pCur->link;
	}

	fclose(fp);
	
}

void newRank(int score){
	Node* pNew;
	Node* pCur=head->link;
	int i;
	char name[NAMELEN]="";
	int flag = 1;

	pNew = (Node*)malloc(sizeof(Node));

	clear();
	echo();
	while(flag)
	{
		printw("your name: ");
		scanw("%s", name);
		if(strcmp(name,"") ==0)
		{
			printw("\nPlease Write Name\n");
			getch();
			clear();
		}
		else
			flag = 0;
	}
	noecho();
	if(head-> num == 0)
	{
		head->link = pNew;
		strcpy(pNew->name,name);
		pNew->link = NULL;
		(head->num)++;
	}
	else
	{
		for(i=0; i< head->num;i++)
		{
			if(score > (pCur->score))
			{
				pNew->score = pCur->score;
				strcpy(pNew->name, pCur->name);
				pCur->score = score;
				strcpy(pCur->name, name);
				pNew->link = pCur->link;
				pCur->link = pNew;
				(head->num)++;
				break;
			}
			if( i == head->num-1)
			{
				pNew->score = score;
				strcpy(pNew->name, name);
				pNew->link = NULL;
				pCur->link = pNew;
				(head->num)++;
				break;
			}
			pCur = pCur->link;
		}
	}

}

void DrawRecommend(int y, int x, int blockID,int blockRotate){
	int i,j;

	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				printw("R");
			}
		}
	move(HEIGHT,WIDTH+10);

}

void ConstructTree(RecNode * root)
{
	RecNode** tmp=root->c;
	int i;

	for(i = 0 ; i < CHILDREN_MAX;i++)
	{	
		tmp[i] = (RecNode*)malloc(sizeof(RecNode));
		tmp[i]->f = (char (*) [WIDTH])malloc(sizeof(char)*HEIGHT*WIDTH);
		tmp[i]->lv = root->lv+1;
		if(tmp[i] ->lv+1 < VISIBLE_BLOCK)
			ConstructTree(tmp[i]);
	}

	return ;
}

void modified_recommend(RecNode *root){
	int r;
	int k = 0;
	int i,j;
	int min_x, max_x;
	int y = -1;
	int flag;
	int score;
	int lv;
	RecNode* cur;
	int DltLine ;
	int tmp;
	int index= 0;

	lv = root->lv + 1; // nextBlock[lv] : 현재 처리하는 블럭 


	for(r=0; r<4;r++)
	{
		//x범위 계산
		flag = 0;
		for(j=0;j<4;j++)
		{
			for(i=0;i<4;i++)
			{
				if(block[nextBlock[lv]][r][i][j] == 1)
					min_x = -1 * j, flag = 1;
			}
			if(flag)
				break;
		}
		flag = 0;
		for(j = 3 ;j>=0 ; j--)
		{
			for(i = 0; i<4;i++)
			{
				if(block[nextBlock[lv]][r][i][j] == 1)
					max_x = WIDTH - 1 - j, flag = 1;
				
			}
			if(flag)
				break;
		} // x범위 계산 완료
		for(k=0 ; k < max_x - min_x +1 ; k++) // c[k] 를 해준다.
		{
			y = -1;
			score = 0;
			DltLine = 0;
			cur = root->c[index++];
			cur->lv = lv;

			for(i = 0 ; i < HEIGHT;i++)
				for(j = 0 ; j < WIDTH ; j++)
					cur->f[i][j] = root->f[i][j]; //field 받는다.

			while(CheckToMove(root->f,nextBlock[lv],r,y+1,min_x+k))
				y++;//넣어질 좌표는 (min_x+k , y)
			if(y == -1)
				continue;
		
			for(i=0;i<BLOCK_HEIGHT;i++)
				for(j=0;j<BLOCK_WIDTH;j++)
					if(block[nextBlock[lv]][r][i][j] == 1)
						cur->f[y+i][min_x+k+j] = 1; 
			//field 완료

			if(cur->lv + 1 < VISIBLE_BLOCK)
			{
				if(lv == 0 )
					tmpR = r, tmpX = min_x+k, tmpY = y;
				modified_recommend(cur);
			}
			else
			{
				tmp = CalWeight(cur);
				if(tmp > max)
				{
					max= tmp;
					recommendR = tmpR, recommendX = tmpX, recommendY = tmpY;
				}
			}

		}
	}
	// user code

	return ;
}

float CalWeight(RecNode* root)
{
	int i,j,k;
	int tmp;
	float height=0,dltLine=0,hole=0,blockade =0;
	float touchB=0, touchF=0, touchW=0;
	
	for(i=0;i<HEIGHT;i++)
	{
		tmp = 0;
		for(j=0;j<WIDTH;j++)
		{
			if(root->f[i][j])
			{
				if(i < HEIGHT -1 && root->f[i+1][j])
					touchB++;
				if(j < WIDTH - 1 && root->f[i][j+1])
					touchB++;
				if(j > 0 && root->f[i][j-1])
					touchB++;
				if(i == HEIGHT-1)
					touchF++;
				if(j == 0 || j == WIDTH -1)
					touchW++;
			}
			if(root->f[i][j])
			{
				height = HEIGHT - i;
				tmp++; //Dlt line 
			}
			if(i < HEIGHT - 1 && root->f[i][j] && ! root->f[i+1][j])
			{
				hole++;
				for(k =1;;k++)
				{
					if(i-k >= 0 && root->f[i-k][j])
						blockade++;
					else
						break;
				}
			}
		}
		if(tmp == WIDTH)
			dltLine++;
	} // height penalty

	return (3 * dltLine - 0.03 * height*height - 7.5 * hole - 3.5 * blockade +3 * touchB + 2.5 * touchW + 5 * touchF) ;


}
void recommendedPlay(){;
	int command;
	clear();
	act.sa_handler = recommendAI;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();

	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}
		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();
			newRank(score);
			writeRankFile();

			return;
		}
		
	}while(!gameOver);

	alarm(0);

	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
	writeRankFile();

	// user code
}

void recommendAI(int sig){
	timed_out=0;
	AddBlockToField(field, nextBlock[0], recommendR,recommendY, recommendX);

	score += DeleteLine(field);
	PrintScore(score);
	nextBlock[0] = nextBlock[1];
	nextBlock[1] = nextBlock[2];
	nextBlock[2] = rand() % 7;
	if(CheckToMove(field,nextBlock[0],0,-1,WIDTH/2-2))
	{
		DrawField();
		DrawNextBlock(nextBlock);
	}
	else
		gameOver=TRUE;

	return ;
}
