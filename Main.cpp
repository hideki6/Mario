//-------------------------------------------------
//  �w�b�_�[�t�@�C���}��
//-------------------------------------------------
#include <math.h>
#include "DxLib.h"

#define WINDOW_WIDTH (1600)
#define WINDOW_HEIGHT (900)

//�p�b�h�̏���񋓌^�Ŏ擾
enum e_But
{
	But_Dow,		//1bit
	But_Lef,		//2bit
	But_Rig,		//3bit
	But_Up,		//4bit
	But_A,		//5bit
	But_B,		//6bit
	But_X,		//7bit
	But_Y,		//8bit
	But_L,		//9bit
	But_R,		//10bit
	But_Sel,		//11bit
	But_Sta,		//12bit
	But_L3,		//13bit
	But_R3,		//14bit
	But_Total
};

//�p�b�h�̃r�b�g���
int Pad_Inp;

//�p�b�h�̉������J�E���g��
int Pad_Cou[e_But::But_Total];

//�p�b�h�̓��͏����擾����֐�
bool Pad_Read()
{
	Pad_Inp = GetJoypadInputState(DX_INPUT_PAD1);

	for (int b = 0; b < e_But::But_Total; b = b + 1)
	{
		if (Pad_Inp & (1 << b))
		{
			Pad_Cou[b] = Pad_Cou[b] + 1;
		}
		else Pad_Cou[b] = 0;
	}
	return FALSE;
}


//�J�������W
int Camera_x;

//�u���b�N�^�C�v��񋓑̂Œ�`
enum e_Type
{
	e_��,
	e_�n��,
	e_����,
	e_�ł�,
	e_�y��,
	e_�n�e�i,
	e_�S�[��,
	e_�N���{�[,
	e_�m�R�m�R,
	e_�}���I,
};

//�ʐ^�̍\���̒�`
struct Pic
{
	int p;			//�ʐ^�n���h��
	int w;			//�ʐ^��
	int t;			//�ʐ^����
};

//�摜�̏�ԗ񋓑̒�`
enum e_Ani
{
	e_Normal,
	e_Walk1,
	e_Walk2,
	e_Jump,
	e_End,
	e_Ani_Total,
};

//�����`�̍\���̒�`
struct Ract
{
	int x;										//x���W
	int y;										//y���W
	int w;										//��
	int t;										//����
	int k;										//���
	double r;									//��]�p�x
	Pic p[e_Ani::e_Ani_Total];						//�ʐ^
	int a;										//�A�j���[�V����
	bool d;										//����
	int j;										//�W�����v�J�E���^
	bool c;										//�L�����N�^�[���ۂ��i�������ۂ��j
};

//�e�L�X�g�t�@�C������u���b�N����ǂݎ�邽�߂̍\���̒�`
struct Text
{
	int k;			//���
	int x;			//x���W
	int y;			//y���W
};

//�e�L�X�g�t�@�C������u���b�N����ǂݎ�邽�߂̕ϐ���`
//�i2�����z��j
Text** Stage;

//�X�e�[�W�̍ő�l
int STAGE_TOTAL;
int STAGE_WIDTH;
int STAGE_HEIGHT;

//�w�i�F�̍\���̒�`
struct Color
{
	int r;		//�ΐ���
	int g;		//�Ԑ���
	int b;		//����
};

//�w�i�F��`
Color Col;

//�u���b�N��`
//�i1�����z��j
Ract* Block;

//�}���I�̃A�h���X
Ract* Mar;

//�u���b�N������
void Block_Ini(Ract* b)
{
	b->x = 0;							//x���W
	b->y = 0;							//y���W
	b->w = 0;							//��
	b->t = e_Type::e_��;					//����
	b->k = 0;							//���
	b->r = 0;							//��]�p�x

	for (int i = 0; i < e_Ani::e_Ani_Total; i = i + 1)
	{
		b->p[i].p = 0;				//�ʐ^�n���h��
		b->p[i].w = 0;				//�ʐ^�̕�
		b->p[i].t = 0;				//�ʐ^�̌���
	}

	b->a = e_Ani::e_Normal;			//�A�j���[�V����
	b->d = FALSE;						//����
	b->j = 0;						//�W�����v�J�E���^
	b->c = FALSE;						//�L�����N�^�[���ۂ�(�������ۂ�)
}

//�W�����v��Ԃ̗񋓑̒�`
enum e_Jump
{
	�n�ʂɒ��n,
	�W�����v��,
	������,
};

//�W�����v�t���O
int Jump;

//�W�����v�J�n����y���W�L�^
int Jump_Ymem;

//�J�E���^
int Cou;

//�Q�[���J�n���̏������s���֐�
void Game_Ini()
{
	//.txt����X�e�[�W�����擾
	char name[] = { "1-1.txt" };
	FILE* fp = NULL;
	errno_t error = fopen_s(&fp, name, "r");

	//�X�e�[�W�����擾����
	if (error == NULL &&
		fp != NULL)
	{
		//�w�i�F�擾
		fscanf_s(fp, "%d\n", &Col.r);
		fscanf_s(fp, "%d\n", &Col.g);
		fscanf_s(fp, "%d\n", &Col.b);
		SetBackgroundColor(Col.r, Col.g, Col.b);

		//�X�e�[�W�̕��~���݁E���E���ݎ擾
		fscanf_s(fp, "%d\n", &STAGE_TOTAL);
		fscanf_s(fp, "%d\n", &STAGE_WIDTH);
		fscanf_s(fp, "%d\n", &STAGE_HEIGHT);

		//�������m�ہi���I�z��j
		Stage = new Text * [STAGE_WIDTH];
		for (int i = 0; i < STAGE_WIDTH; i = i + 1)
		{
			Stage[i] = new Text[STAGE_HEIGHT];
		}

		//�u���b�N��x�ԍ��Ey�ԍ��E��ޓǂݍ���
		for (int y = 0; y < STAGE_HEIGHT; y = y + 1)
		{
			for (int x = 0; x < STAGE_WIDTH; x = x + 1)
			{
				fscanf_s(fp, "%d", &Stage[x][y].k);
				Stage[x][y].x = x;
				Stage[x][y].y = y;
			}
		}
		//�t�@�C������
		fclose(fp);
	}

	//�������m��
	Block = new Ract[STAGE_TOTAL];

	//Text�^����Ract�^�ɕϊ����Atxt�̏���Block[]�Ɋi�[
	int i = 0;
	for (int y = 0; y < STAGE_HEIGHT; y = y + 1)
	{
		for (int x = 0; x < STAGE_WIDTH; x = x + 1)
		{
			if (Stage[x][y].k == e_Type::e_��)		//�u���b�N�Ȃ��̏ꍇ
			{
				//�u���b�N������
				Block_Ini(&Block[i]);
			}
			else														//�u���b�N�݂̂̏ꍇ
			{
				//�u���b�N�̕��ƌ��݂��u���b�N�̎�ނ��画�f
				switch (Stage[x][y].k)
				{
				case e_Type::e_�n��:

					//�u���b�N�̎ʐ^���
					Block[i].p[0].p = 
					LoadGraph("picture/jimen_block.png");
						
					//����
					Block[i].d = FALSE;

					//�L�����N�^�[���ۂ�
					Block[i].c = FALSE;

					break;

				case e_Type::e_����:
					//�u���b�N�̎ʐ^���
					Block[i].p[0].p = LoadGraph("picture/renga_block.png");

					//����
					Block[i].d = FALSE;

					//�L�����N�^�[���ۂ�
					Block[i].c = FALSE;

					break;

				case e_Type::e_�ł�:
					//�u���b�N�̎ʐ^���
					Block[i].p[0].p = LoadGraph("picture/hatena_block.png");

					//����
					Block[i].d = FALSE;

					//�L�����N�^�[���ۂ�
					Block[i].c = FALSE;

					break;

				case e_Type::e_�y��:
					//�u���b�N�̎ʐ^���
					Block[i].p[0].p = LoadGraph("picture/dokan.png");

					//����
					Block[i].d = FALSE;

					//�L�����N�^�[���ۂ�
					Block[i].c = FALSE;

					break;

				case e_Type::e_�n�e�i:
					//�u���b�N�̎ʐ^���
					Block[i].p[0].p = LoadGraph("picture/hatena_block.png");

					//����
					Block[i].d = FALSE;

					//�L�����N�^�[���ۂ�
					Block[i].c = FALSE;

					break;

				case e_Type::e_�S�[��:
					//�u���b�N�̎ʐ^���
					Block[i].p[0].p = LoadGraph("picture/hatena_block.png");

					//����
					Block[i].d = FALSE;

					//�L�����N�^�[���ۂ�
					Block[i].c = FALSE;

					break;

				case e_Type::e_�N���{�[:
					//�u���b�N�̎ʐ^���
					Block[i].p[0].p = LoadGraph("picture/kuribo.png");

					//����
					Block[i].d = TRUE;

					//�L�����N�^�[���ۂ�
					Block[i].c = TRUE;

					break;
//#ifdef false
				case e_Type::e_�m�R�m�R:
					//�u���b�N�̎ʐ^���
					Block[i].p[0].p = LoadGraph("picture/hatena_block.png");

					//����
					Block[i].d = FALSE;

					//�L�����N�^�[���ۂ�
					Block[i].c = TRUE;

					break;
//#endif
				case e_Type::e_�}���I:
					//�u���b�N�̎ʐ^���
					Block[i].p[e_Ani::e_Normal].p = LoadGraph("picture/neutral_mario.png");
					Block[i].p[e_Ani::e_Walk1].p = LoadGraph("picture/aruku_mario.png");
					Block[i].p[e_Ani::e_Walk2 ].p = LoadGraph("picture/aruku2_mario.png");
					Block[i].p[e_Ani::e_Jump].p = LoadGraph("picture/jump_mario.png");
					Block[i].p[e_Ani::e_End].p = LoadGraph("picture/miss_mario.png");

					//����
					Block[i].d = FALSE;

					//�L�����N�^�[���ۂ�
					Block[i].c = TRUE;

					break;
				}

				//�u���b�N�ʐ^������݂ƕ��擾
				GetGraphSize(
					Block[i].p[0].p,
					&Block[i].p[0].w,
					&Block[i].p[0].t);

				//�u���b�N�̕��ƌ��݂���
				Block[i].w = Block[i].p[0].w;
				Block[i].t = Block[i].p[0].t;

				//�u���b�N�̍��W�E��ށE��]�p�x�E�W�����v�J�E���^����
				Block[i].x = Stage[x][y].x * 50;
				Block[i].y = Stage[x][y].y * 50;
				Block[i].k = Stage[x][y].k;
				Block[i].r = 0.;
				Block[i].j = 0;
				Block[i].a = e_Ani::e_Normal;
			}

			//���[�v�J�E���^
			i = i + 1;
		}
	}

	//����������iText�^ Stage�j
	for (int i = 0; i < STAGE_WIDTH; i = i + 1)
	{
		delete[] Stage[i];
	}
	delete[] Stage;

	//������
	Camera_x = 0;

	//�W�����v���̕ϐ�������
	Jump = e_Jump::�n�ʂɒ��n;

	//�J�E���^
	Cou = 0;
}

//�Q�[���I�����̏������s���֐�
void Game_End() {}

//�}���I�̈ړ���
#define Move_X (3)
#define Move_Max_Y (300)

//--------------------------------
//�T�@�v�F�ڐG������s���֐�
//���@���F�L�����A�L�����̈��q�A�ڐG���ɓ|�����ۂ�
//�߂�l�F�ڐG���茋��
//--------------------------------
bool HitJudg(Ract c, int i_c, bool d)
{
	for (int i = 0; i < STAGE_TOTAL; i = i + 1)
	{
		if (i != i_c)	//�������̓��m�̔�r������邽��
		{
			if (((Block[i].x <= c.x &&
				c.x < (Block[i].x + Block[i].w)) ||

				(Block[i].x <= c.x + c.w &&
				c.x + c.w < Block[i].x + Block[i].w)) &&

				((Block[i].y <= c.y &&
				c.y < Block[i].y + Block[i].t) ||

				(Block[i].y <= c.y + c.t &&
				c.y + c.t < Block[i].y + Block[i].t)))
			{
#ifdef false
				//�}���I���G�L�����ƐڐG�����ꍇ
				if ((c.k == e_Type::e_�}���I || Block[i].k == e_Type::e_�}���I) &&
					(Block[i].c == TRUE))
#endif
				//�}���I���G�L�����ƐڐG�����ꍇ�i�����̍\���̂��L�����N�^�[�Ƃ��Đݒ肳��Ă���ꍇ�̂ݎg�p�j
				if ((c.k == e_Type::e_�}���I || Block[i].k == e_Type::e_�}���I) &&
					(c.c == TRUE) &&
					(Block[i].c == TRUE))

				{
					//�ڐG���ɓG�L������|�����ۂ�
					switch (d)
					{
					case FALSE:
						Game_End();			//�Q�[���I�����̏���(�Q�[���I�[�o�[)
						break;

					case TRUE:
						Block_Ini(&Block[i]);	//�u���b�N������(�G�L�����|��)
						break;
					}
				}
				//�S�[���ɐڐG�����ꍇ
				else if (c.k == e_Type::e_�}���I &&
					Block[i].k == e_Type::e_�S�[��)
				{
					Game_End();				//�Q�[���I�����̏���(�Q�[���N���A)
				}

				return TRUE;	//�ڐG
			}
		}
	}
	return FALSE;	//���ڐG
}

//�Q�[���v�Z�������s���֐�
void Game_Cal()
{
	//�}���Iy�����ړ��ʒ�`
	int y_add = 0;

	//�W�����v���̔��萔
	const int a = 25;

	//�}���I�̈���
	int c_i = 0;

	//�G�L�����N�^�[�̈ړ�
	for (int i = 0; i < STAGE_TOTAL; i = i + 1)
	{
		//�G�L�����T��
		if (Block[i].k == e_Type::e_�N���{�[ ||
			Block[i].k == e_Type::e_�m�R�m�R)
		{
			//printf("%b\n", &Block[i].d);
			//�G�L�����N�^�[x�����ړ��v�Z
			for (int x = 0; x < Move_X; x = x + 1)
			{
				//�G�L�����N�^�[�ړ�
				switch (Block[i].d)
				{
				case FALSE:	//�E����
					Block[i].x = Block[i].x + 1;
					break;
				case TRUE:	//������
					Block[i].x = Block[i].x - 1;
					break;
				}
//#ifdef false
				//x�����̓G�L�����ƃu���b�N�̐ڐG����
				if (HitJudg(Block[i], i, FALSE) == TRUE)
				{
					//�G�L�����N�^�[���]�ƈړ�
					switch (Block[i].d)
					{
					case FALSE:	//�E����
						Block[i].d = TRUE;
						Block[i].x = Block[i].x + 1;
						break;

					case TRUE:	//������
						Block[i].d = FALSE;
						Block[i].x = Block[i].x - 1;
						break;
					}

					//���[�v����
					break;
				}
//#endif
			}


			Block[i].j = Block[i].j + 1;

			//�G�L�����N�^�[y�����ړ��ʌv�Z
			y_add = (Block[i].j * Block[i].j) / a;

			//�G�L�����N�^�[���R�����v�Z
			for (int y = 0; y < y_add; y = y + 1)
			{
				//�G�L�����N�^�[y���W�v�Z
				Block[i].y = Block[i].y + 1;

				//y�����̓G�L�����ƃu���b�N�̐ڐG����
				if (Block[i].y > WINDOW_HEIGHT ||
					HitJudg(Block[i], i, FALSE) == TRUE)
				{
					//�G�L�����N�^�[y���W�v�Z
					Block[i].y = Block[i].y - 1;

					//�W�����v�J�E���^������
					Block[i].j = 0;

					//���[�v����
					break;
				}
			}
		}
	}

	//�}���I���u���b�N����T�����ăR�s�[
	int m_x = 0;
	for (int i = 0; i < STAGE_TOTAL; i = i + 1)
	{
		if (Block[i].k == e_Type::e_�}���I)
		{
			//�}���I�R�s�[
			Mar = &Block[i];

			//�}���I���W�R�s�[
			m_x = Block[i].x;

			//�}���I�̈���
			c_i = i;

			//���[�v����
			break;
		}
	}

	//�}���I�E�ړ�(�������Ή�)
	if (Pad_Cou[e_But::But_Rig] != 0)
	{
		//�}���I�E����������
		Mar->d = FALSE;

		for (int i = 0; i < Move_X; i = i + 1)
		{
			//�}���Ix���W�v�Z
			Mar->x = Mar->x + 1;

			//�J����x���W�v�Z
			if (Mar->x > WINDOW_WIDTH / 2 &&
				Mar->x < STAGE_WIDTH * 50 - WINDOW_WIDTH / 2)
			{
				Camera_x = Camera_x - 1;
			}

			//x�����̃}���I�ƃu���b�N�̐ڐG����
			if (Mar->x > STAGE_WIDTH * 50 - Mar->w ||
				HitJudg(*Mar, c_i, FALSE) == TRUE)
			{
				//�}���Ix���W�v�Z
				Mar->x = Mar->x - 1;

				//�J����x���W�v�Z
				if (Mar->x > WINDOW_WIDTH / 2 &&
					Mar->x < STAGE_WIDTH * 50 - WINDOW_WIDTH / 2)
				{
					Camera_x = Camera_x + 1;
				}

				//���[�v����
				break;
			}
		}
	}

	//�}���I���ړ�(�������Ή�)
	if (Pad_Cou[e_But::But_Lef] != 0)
	{
		//�}���I������������
		Mar->d = TRUE;

		for (int i = 0; i < Move_X; i = i + 1)
		{
			//�}���Ix���W�v�Z
			Mar->x = Mar->x - 1;

			//�J����x���W�v�Z
			if (Mar->x > WINDOW_WIDTH / 2 &&
				Mar->x < STAGE_WIDTH * 50 - WINDOW_WIDTH / 2)
			{
				Camera_x = Camera_x + 1;
			}

			//x�����̃}���I�ƃu���b�N�̐ڐG����
			if (Mar->x < 0 ||
				HitJudg(*Mar, c_i, FALSE) == TRUE)
			{
				//�}���Ix���W�v�Z
				Mar->x = Mar->x + 1;

				//�J����x���W�v�Z
				if (Mar->x > WINDOW_WIDTH / 2 &&
					Mar->x < STAGE_WIDTH * 50 - WINDOW_WIDTH / 2)
				{
					Camera_x = Camera_x - 1;
				}

				//���[�v����
				break;
			}
		}
	}

	//�}���I�W�����v
	if (Pad_Cou[e_But::But_Up] == 1 &&
		Jump == e_Jump::�n�ʂɒ��n)
	{
		//�W�����v�t���O�ύX
		Jump = e_Jump::�W�����v��;

		//�W�����v���̃}���I���W�L�^
		Jump_Ymem = Mar->y;
	}

	//�W�����v���̍��W�v�Z
	if (Jump == e_Jump::�W�����v��)
	{
		//�W�����v�J�E���^
		Mar->j = Mar->j + 1;

		//�}���Iy�����ړ��ʌv�Z
		y_add = (Mar->j * Mar->j) / a;

		for (int i = 0; i < y_add; i = i + 1)
		{
			//�}���I��y���W�v�Z
			Mar->y = Mar->y - 1;

			//y�����̃}���I�ƃu���b�N�̐ڐG����
			if (Mar->y < 0 ||
				HitJudg(*Mar, c_i, FALSE) == TRUE ||
				Move_Max_Y == Jump_Ymem - Mar->y)
			{
				//�W�����v�t���O�ύX
				Jump = e_Jump::������;

				//�}���Iy���W�v�Z
				Mar->y = Mar->y + 1;

				//�W�����v�J�E���^������
				Mar->j = 0;

				//���[�v����
				break;
			}
		}
	}

	//�}���I���R����
	if (Jump == e_Jump::������ ||
		Jump == e_Jump::�n�ʂɒ��n)
	{
		//�W�����v�J�E���^
		Mar->j = Mar->j + 1;

		//�}���Iy�����ړ��ʌv�Z
		y_add = (Mar->j * Mar->j) / a-3;

		for (int i = 0; i < y_add; i = i + 1)
		{
			//�}���Iy���W�v�Z
			Mar->y = Mar->y + 1;

			//y�����̃}���I�ƃu���b�N�̐ڐG����
			if (Mar->y > WINDOW_HEIGHT ||
				HitJudg(*Mar, c_i, TRUE) == TRUE)
			{
				//�W�����v�t���O�ύX
				Jump = e_Jump::�n�ʂɒ��n;

				//�}���Iy���W�v�Z
				Mar->y = Mar->y - 1;

				//�W�����v�J�E���^������
				Mar->j = 0;

				//���[�v����
				break;
			}
		}
	}

	Cou = Cou + 1;
	if (Cou == 60) Cou = 0;

	//�}���I�A�j���[�V����
	if (Jump == e_Jump::�W�����v�� ||
		Jump == e_Jump::������)
	{
		Mar->a = e_Ani::e_Jump;
	}
	else if (m_x != Mar->x)
	{
		if (0 <= Cou && Cou < 10)Mar->a = e_Ani::e_Normal;
		else if (10 <= Cou && Cou < 20)Mar->a = e_Ani::e_Walk1;
		else if (20 <= Cou && Cou < 30)Mar->a = e_Ani::e_Walk2;
		else if (30 <= Cou && Cou < 40)Mar->a = e_Ani::e_Normal;
		else if (40 <= Cou && Cou < 50)Mar->a = e_Ani::e_Walk1;
		else if (50 <= Cou && Cou < 26)Mar->a = e_Ani::e_Walk2;
	}
	else Mar->a = e_Ani::e_Normal;
}

//�Q�[���`�揈�����s���֐�
void Game_Draw()
{
	//�u���b�N�`��
	for (int i = 0; i < STAGE_TOTAL; i = i + 1)
	{
		//�}���I�`��
		if (Block[i].k == e_Type::e_�}���I)
		{
			switch (Mar->d)
			{
			case FALSE:		//�E��������
				DrawGraph(
					Mar->x + Camera_x,
					Mar->y,
					Mar->p[Mar->a].p,
					TRUE);

				break;

			case TRUE:		//����������
				DrawTurnGraph(
					Mar->x + Camera_x,
					Mar->y,
					Mar->p[Mar->a].p,
					TRUE);

				break;
			}
		}

		//�}���I�ȊO�`��
		else if(Block[i].k != e_Type::e_��)
		{
			switch (Block[i].d)
			{
			case FALSE:		//�E��������
				DrawTurnGraph(
					Block[i].x + Camera_x,
					Block[i].y,
					Block[i].p[0].p,
					TRUE);

				break;

			case TRUE:		//����������
				DrawGraph(
					Block[i].x + Camera_x,
					Block[i].y,
					Block[i].p[0].p,
					TRUE);

				break;
			}
		}

	}
}

//�L�[�̏���񋓌^�Ŏ擾
enum e_Key
{
	key_Dow,	//1bit
	Key_Lef,	//2bit
	Key_Rig,	//3bit
	Key_Up,		//4bit
	Key_A,		//5bit
	Key_D,		//6bit
	Key_W,		//7bit
	Key_Spa,	//8bit
	Key_Total
};

//�L�[�̃r�b�g���
int Key_Inp;

//�L�[�̉������J�E���g��
int Key_Cou[e_Key::Key_Total];

#ifdef false
//�L�[�̓��͏����擾����֐�
bool Key_Read()
{
		BYTE key[256];
		GetKeyboardState(key);

		for (int k = 0; k < e_Key::Key_Total; k = k + 1)
		{
			if (Key_Inp & (1 << k))
			{
				Key_Cou[k] = Key_Cou[k] + 1;
			}
			else Key_Cou[k] = 0;
	}
	return FALSE;
}
#endif


//-------------------------------------------------
//  �T�v�F���C���֐�
//  �����F-
//  �߂�l�F����/�ُ�
//-------------------------------------------------
static int mStartTime;		//����J�n����
static int mCount;			//�J�E���^
static float mFps;			//fps
static const int N = 60;		//���ς����T���v����
static const int FPS = 60;		//�ݒ肵��FPS

void Update()
{
	if (mCount == 0)
	{
		mStartTime = GetNowCount();
	}
	if (mCount == N)
	{
		int t = GetNowCount();
		mFps = 1000.f / ((t - mStartTime) / (float)N);
		mCount = 0;
		mStartTime = t;
	}
	mCount++;
}

void Wait()
{
	int tookTime = GetNowCount() - mStartTime;			//������������
	int waitTime = mCount * 1000 / FPS - tookTime;		//�҂ׂ�����
	if (waitTime > 0)
	{
		Sleep(waitTime);		//�ҋ@
	}
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{

	ChangeWindowMode(TRUE);
	DxLib_Init();

	//�E�B���h�E������
	SetMainWindowText("�X�[�p�[�}���I");
	SetWindowInitPosition(0, 0);						//�w�i��ʂ̏����ݒ�
	SetGraphMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32);		//�w�i��ʂ̕��ƍ���
	SetDrawScreen(DX_SCREEN_BACK);

	Game_Ini();

	//�P�b�ԂɂU�O��i�e�o�r��j�������[�v
	while (ScreenFlip() == 0 &&
		ClearDrawScreen() == 0 &&
		ProcessMessage() == 0 &&	//��O�͐�΂ɏ������
		Pad_Read() == FALSE)
	{
		Update();
		Game_Cal();
		Game_Draw();
		Wait();
	}

	DxLib_End();
	return 0;
}


