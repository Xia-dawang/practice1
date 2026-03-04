#include <graphics.h>
#include <string>
#include <vector>
/// <summary>
/// 基础框架
/// 1. 主循环 -> 事件循环 -> 批量绘图
/// 2. 动态延时
/// </summary>
/// <returns></returns>
/// 

int anim_current_index = 0;
int const PLAYER_ANIM_NUM = 6;

int windowW = 1280;
int windowH = 720;
//IMAGE img_player_left[PLAYER_ANIM_NUM];
//IMAGE img_player_right[PLAYER_ANIM_NUM];

IMAGE img_enemy_left[PLAYER_ANIM_NUM];
IMAGE img_enemy_right[PLAYER_ANIM_NUM];

//AlphaBlend在MSIMG32.LIB这个库里。
#pragma comment(lib, "MSIMG32.LIB")

inline void putimage_alpha(int x, int y, IMAGE* img)
{
	//-> : 解引用，使用这个指针访问变量的成员。
	int w = img->getwidth();
	int h = img->getheight();
	//GetImageHDC(NULL), x, y, w, h
	//这个是目标，GetImageHDC(NULL)是获取目标画布，后面是坐标和宽高
	// GetImageHDC(NULL)表示当前绘图窗口，是EasyX的约定
	//GetImageHDC(img), 0, 0, w, h
	//这个是图片源，从0，0开始提取，提取到对应宽高。
	//AC_SRC_OVER：将源图片叠在目标图片上
	//AC_SRC_ALPHA： 告诉系统使用图片每个像素自己的Alpha通道来做透明混合
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA });
}



void LoarAnimation()
{
	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++)
	{
		//std::wstring path = L"img/player_left_" + std::to_wstring(i) + L".png";
		////c_str()是把c++中的std::string和std::wstring转换成对应C风格的指针。
		//loadimage(&img_player_left[i], path.c_str());
	}

	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++)
	{
		/*std::wstring path = L"img/player_right_" + std::to_wstring(i) + L".png";
		loadimage(&img_player_right[i], path.c_str());*/
	}
}

class Animation
{
public:
	Animation(LPCTSTR path, int num, int interval)
	{
		interval_ms = interval;

		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++)
		{
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);

			frame_list.push_back(frame);
		}
	}

	~Animation()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
		{
			delete frame_list[i];
		}
	}

	void Play(int x, int y, int delta)
	{
		timer += delta;
		if (timer >= interval_ms)
		{
			idx_frame = (idx_frame + 1) % frame_list.size();
			timer = 0;
		}
		putimage_alpha(x, y, frame_list[idx_frame]);
	}

private:
	int timer = 0;			//动画计时器
	int idx_frame = 0;		//动画帧索引
	int interval_ms = 0;
	std::vector<IMAGE*> frame_list;
};

class Player
{
public:
	Player()
	{
		player_shadow = loadimage(&player_shadow, _T("img/shadow_player.png"));
		anim_left = new Animation(_T("img/player_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/player_right_%d.png"), 6, 45);
	}

	~Player()
	{
		delete anim_left;
		delete anim_right;
	}

	/// <summary>
	/// 处理玩家的操作信息
	/// </summary>
	/// <param name="msg"></param>
	void ProcessEvent(const ExMessage& msg)
	{
		//if (msg.message == WM_KEYDOWN)
		switch (msg.message)
		{
		case WM_KEYDOWN:
		{
			switch (msg.vkcode)
			{
			case VK_UP:
				moveUp = true;
				break;
			case VK_DOWN:
				moveDown = true;
				break;
			case VK_LEFT:
				moveLeft = true;
				break;
			case VK_RIGHT:
				moveRight = true;
				break;
			default:
				break;
			}
			break;
		}
		case WM_KEYUP:
		{
			switch (msg.vkcode)
			{
			case VK_UP:
				moveUp = false;
				break;
			case VK_DOWN:
				moveDown = false;
				break;
			case VK_LEFT:
				moveLeft = false;
				break;
			case VK_RIGHT:
				moveRight = false;
				break;
			default:
				break;
			}
			break;
		}
		}
	}

	/// <summary>
	/// 处理玩家移动
	/// </summary>
	void Move()
	{
		int dir_x = moveRight - moveLeft;
		int dir_y = moveDown - moveUp;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			player_pos.x += (int)(PLAYER_SPEED * normalized_x);
			player_pos.y += (int)(PLAYER_SPEED * normalized_y);
		}

		//校准玩家的位置
		if (player_pos.x < 0) player_pos.x = 0;
		if (player_pos.y < 0) player_pos.y = 0;
		if (player_pos.x > windowW - PLAYER_WIDTH) player_pos.x = windowW - PLAYER_WIDTH;
		if (player_pos.y > windowH - PLAYER_HEIGHT) player_pos.y = windowH - PLAYER_HEIGHT;
	}

	/// <summary>
	/// 处理绘制玩家
	/// </summary>
	/// <param name="delta"></param>
	void Draw(int delta)
	{
		int pos_shadow_x = player_pos.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = player_pos.y + (PLAYER_HEIGHT - 12);
		putimage_alpha(pos_shadow_x, pos_shadow_y, &player_shadow);

		static bool facing_left = false;
		if (moveRight - moveLeft < 0)
		{
			facing_left = true;
		}
		else if (moveRight - moveLeft > 0)
		{
			facing_left = false;
		}

		if (facing_left)
		{
			anim_left->Play(player_pos.x, player_pos.y, delta);
		}
		else
		{
			anim_right->Play(player_pos.x, player_pos.y, delta);
		}
	}

	POINT GetPosition() const
	{
		return player_pos;
	}

	int GetWeight() const
	{
		return PLAYER_WIDTH;
	}

	int GetHeight() const
	{
		return PLAYER_HEIGHT;
	}

private:
	const int PLAYER_SPEED = 10;
	const int PLAYER_WIDTH = 80;
	const int PLAYER_HEIGHT = 80;
	const int SHADOW_WIDTH = 32;

private:
	IMAGE player_shadow;
	POINT player_pos = { 500, 500 };
	Animation* anim_left;
	Animation* anim_right;

	bool moveUp = false;
	bool moveDown = false;
	bool moveLeft = false;
	bool moveRight = false;

};



void DrawPlayer(int delta, int dir_x)
{
	
}

class Bullet
{
public:
	POINT position = { 0, 0 };

public:
	Bullet() = default;
	~Bullet() = default;

	void Draw() const
	{
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(200, 75, 10));
		fillcircle(position.x, position.y, RADIUS);
	}
private:
	const int RADIUS = 10;
};

class Enemy
{
public:
	Enemy()
	{
		loadimage(&enemy_shadow, _T("img/shadow_enemy.png"));
		anim_left = new Animation(_T("img/enemy_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/enemy_right_%d.png"), 6, 45);

		enum class SpawnEdge
		{
			Up = 0,
			Down,
			Left,
			Right
		};

		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge)
		{
		case SpawnEdge::Up:
			position.x = rand() % windowW;
			position.y = -FRAME_HEIGHT;
			break;
		case SpawnEdge::Down:
			position.x = rand() % windowW;
			position.y = windowH;
			break;
		case SpawnEdge::Left:
			position.x = -FRAME_WIDTH;
			position.y = rand() % windowH;
			break;
		case SpawnEdge::Right:
			position.x = windowW;
			position.y = rand() % windowH;
			break;
		default:
			break;
		}
	}

	bool CheckBulletCollision(const Bullet& bullet)
	{
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + FRAME_WIDTH;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + FRAME_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}

	//为了追求简单，将enemy的中心点作为碰撞点，而不是两个矩形是否有交点作为碰撞条件
	bool CheckPlayerCollision(const Player& player)
	{
		POINT player_pos = player.GetPosition();
		POINT enemy_center = { position.x + FRAME_WIDTH / 2, position.y + FRAME_HEIGHT / 2 };
		bool is_overlap_x = enemy_center.x >= player_pos.x && enemy_center.x <= player_pos.x + player.GetWeight();
		bool is_overlap_y = enemy_center.y >= player_pos.y && enemy_center.y <= player_pos.y + player.GetHeight();
		return is_overlap_x && is_overlap_y;
	}
	
	void Draw(int delta)
	{
		int pos_shadow_x = position.x + (FRAME_WIDTH - SHADOW_WIDTH) / 2;
		int pos_shadow_y = position.y;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &enemy_shadow);

		if (facing_left)
		{
			anim_left->Play(position.x, position.y, delta);
		}
		else
		{
			anim_right->Play(position.x, position.y, delta);
		}
	}

	void Move(const Player& player)
	{
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalize_x = dir_x / len_dir;
			double normalize_y = dir_y / len_dir;
			position.x += (int)(normalize_x * SPEED);
			position.y += (int)(normalize_y * SPEED);
		}

		if (dir_x > 0)
		{
			facing_left = false;
		}
		else if (dir_x < 0)
		{
			facing_left = true;
		}
	}

	~Enemy()
	{
		delete anim_left;
		delete anim_right;
	}

	

private:
	const int SPEED = 2;
	const int FRAME_WIDTH = 80;		//敌人的宽度
	const int FRAME_HEIGHT = 80;	//敌人的高度
	const int SHADOW_WIDTH = 48;	//阴影的宽度

private:
	IMAGE enemy_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0,0 };
	bool facing_left = false;
};



void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)
{
	const int INTERVAL = 100;
	static int counter = 0;
	if ((++counter) % INTERVAL == 0)
		enemy_list.push_back(new Enemy);
}

//更新子弹位置
void UpdateBullets(std::vector<Bullet>& bullet_list, const Player& player)
{
	const double RADIAL_SPEED = 0.0045;							//径向波动速度；
	const double TANGENT_SPEED = 0.0055;						//切向波动速读；
	double radian_interval = 2 * 3.14159 / bullet_list.size();	//子弹间的弧度间隔
	POINT player_pos = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);
	for (size_t i = 0; i < bullet_list.size(); i++)
	{
		double radian = GetTickCount() * TANGENT_SPEED + radian_interval;	//当前子弹所在弧度值
		bullet_list[i].position.x = player_pos.x + player.GetWeight() / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_pos.y + player.GetHeight() / 2 + (int)(radius * cos(radian));
	}
}


int main()
{

	bool isContinue = true;

	ExMessage msg;
	IMAGE img_background;
	Player player;

	std::vector<Enemy*> enemy_list;
	std::vector<Bullet> bullet_list(3);
	//bool moveUp = false;
	//bool moveDown = false;
	//bool moveLeft = false;
	//bool moveRight = false;

	loadimage(&img_background, _T("img/background.png"));
	//loadimage(&player_shadow, _T("img/shadow_player.png"));
	//LoarAnimation();
	initgraph(windowW, windowH);
	BeginBatchDraw();
	
	while (isContinue)
	{
		DWORD start_time = GetTickCount();
		while (peekmessage(&msg))
		{
			player.ProcessEvent(msg);
		}
		/*if (moveUp) player_pos.y -= PLAYER_SPEED;
		if (moveDown) player_pos.y += PLAYER_SPEED;
		if (moveLeft) player_pos.x -= PLAYER_SPEED;
		if (moveRight) player_pos.x += PLAYER_SPEED;*/

		/*static int counter = 0;
		if (++counter % 5 == 0)
			anim_current_index++;
		anim_current_index = anim_current_index % PLAYER_ANIM_NUM;*/

		

		cleardevice();
		putimage(0, 0, &img_background);
		player.Move();
		UpdateBullets(bullet_list, player);
		TryGenerateEnemy(enemy_list);
		for (Enemy* enemy : enemy_list)
		{
			enemy->Move(player);
		}

		for (Enemy* enemy : enemy_list)
		{
			if (enemy->CheckPlayerCollision(player))
			{
				MessageBox(GetHWnd(), _T("扣1观看战败CG"), _T("游戏结束"), MB_OK);
				isContinue = false;
				break;
			}
		}
		//直接使用putimage会有黑框。因为这个函数没有封装透明度信息，需要自己写。
		//putimage_alpha(player_pos.x, player_pos.y, &img_player_left[anim_current_index]);
		//DrawPlayer(1000/30, moveRight - moveLeft);
		player.Draw(1000 / 30);
		for (Enemy* enemy : enemy_list)
		{
			enemy->Draw(1000 / 30);
		}
		for (const Bullet& bullet : bullet_list)
		{
			bullet.Draw();
		}
		
		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time -  start_time;
		if (delta_time < 1000 / 60)
		{
			Sleep(1000 / 60 - delta_time);
		}
	}

	EndBatchDraw();
}