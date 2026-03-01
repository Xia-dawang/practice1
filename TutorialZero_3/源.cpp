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

int main()
{

	bool isContinue = true;

	ExMessage msg;
	IMAGE img_background;
	Player player;

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
		//直接使用putimage会有黑框。因为这个函数没有封装透明度信息，需要自己写。
		//putimage_alpha(player_pos.x, player_pos.y, &img_player_left[anim_current_index]);
		//DrawPlayer(1000/30, moveRight - moveLeft);
		player.Draw(1000 / 30);
		
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