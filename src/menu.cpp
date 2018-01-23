#include "menu.h"

Menu menu;

#define StartColor			D3DCOLOR_ARGB(255, 33, 33, 33)
#define EndColor			D3DCOLOR_ARGB(255, 20, 20, 20)
#define BorderColor			D3DCOLOR_ARGB(255, 0, 0, 0)
#define SelectionColor			D3DCOLOR_ARGB(255, 13, 13, 13)
#define OffColor			D3DCOLOR_ARGB(255, 135, 135, 135)
#define ColorOffset			D3DCOLOR_ARGB(0, 25, 25, 25)

Menu::Menu()
{
	// menu
	this->menuInfo.x		= 50;	// posição x do menu
	this->menuInfo.y		= 150;	// posição y do menu
	this->menuInfo.w		= 300;	// largura do menu
	this->menuInfo.isShown	= true;	// visibilidade do menu

	// item
	this->itemInfo.num		= 0;	// número de itens
	this->itemInfo.index	= 0;	// índice atual

	// tab
	this->tabInfo.num		= 0;	// número de abas
	this->tabInfo.index		= 0;	// aba atual

	// scroll
	this->scrollInfo.num	= 7;	// quantidade de itens exibidos
	this->scrollInfo.index	= 0;	// índice do scroll

	// color
	this->colorInfo.num		= 0;	// número de cores
	this->colorInfo.index	= 0;	// índice da cor
}

void Menu::AddTab(string name)
{
	this->tabs[this->tabInfo.num].name = name;
	this->tabs[this->tabInfo.num].pos = itemInfo.num;
	this->tabInfo.num++;
}

void Menu::AddItem(string name, string* display, int max, int value)
{
	this->items[this->itemInfo.num].name = name;
	this->items[this->itemInfo.num].display = display;
	this->items[this->itemInfo.num].max = max - 1;
	this->items[this->itemInfo.num].value = value;
	this->itemInfo.num++;
}

void Menu::Render()
{
	if (GetAsyncKeyState(VK_INSERT) &1)
		this->menuInfo.isShown = !this->IsShown();

	if (!this->IsShown())
		return;

	// propriedades
	int x = this->menuInfo.x;					// posição x
	int y = this->menuInfo.y;					// posição y
	int w = this->menuInfo.w;					// largura
	int h = (this->scrollInfo.num * 16) + 8;	// altura

	// altura dividida pelo número de abas
	int tabHeight = h/this->tabInfo.num;
	// largura da aba representa 1/4 da largura total - equivalente a 25%
	int tabWidth = w/4;

	// retorna a posição do cursor de acordo com a janela
	GetCursorPos(&this->c.pos);
	ScreenToClient(GetActiveWindow(), &this->c.pos);

	// navegação
	this->MouseSetup();
	this->MouseScrollNavigation(x + tabWidth + 5, y + 25, w - tabWidth - 20, (this->scrollInfo.num * 16));
	this->KeyboardNavigation();

	// mover pelo cabeçalho
	this->DragMenu(x, y, w, 20);

	// cabeçalho
	draw.Rectangle(x, y, w, 20, StartColor, EndColor, BorderColor);
	draw.Text(x + w/2, y + 2, this->menuInfo.title, OffColor, false, TextAlignment::kCenter);

	// preencher o menu de acordo com a largura das abas
	draw.Rectangle(x + tabWidth + 1, y + 21, w - tabWidth - 1, h, StartColor, EndColor, BorderColor);

	// rodapé
	draw.Rectangle(x, y + h + 22, w, 20, StartColor, EndColor, BorderColor);

	// componentes
	this->DrawSelection(x + tabWidth + 5, y + 20 + (this->itemInfo.index - this->scrollInfo.index) * 16 + 6, w - tabWidth - 20);
	this->DrawTabs(x, y + 21, tabWidth, tabHeight);
	this->DrawItems(x + tabWidth + 10, y + 20, w);
	this->DrawScrollBar(x + w - 10, y + 27, h - 15);

	// cores
	this->DrawColors(x, y, w, h + 28);

	//cursor
	draw.Rectangle(c.pos.x, c.pos.y, 10, 10, StartColor, EndColor, BorderColor);
}

void Menu::MouseSetup()
{
	this->c.isLClicked = GetAsyncKeyState(VK_LBUTTON) &1 ? true : false;
	this->c.isRClicked = GetAsyncKeyState(VK_RBUTTON) &1 ? true : false;
	this->c.isHolding  = GetAsyncKeyState(VK_LBUTTON) ? true : false;
}

void Menu::MouseScrollNavigation(float x, float y, float w, float h)
{
	if (this->IsMouseOver(x, y, w, h))
	{
		if (input.IsScrollUp() && this->scrollInfo.index > 0)
		{
			this->scrollInfo.index--;
			this->itemInfo.index--;
		}

		if (input.IsScrollDown() && this->scrollInfo.index < this->NumScrollItems())
		{
			this->scrollInfo.index++;
			this->itemInfo.index++;
		}
	}
}

void Menu::KeyboardNavigation()
{
	// fixar índice dos itens para alterar seu valor
	int index = this->FirstTabIndex() + this->itemInfo.index;

	if (GetAsyncKeyState(VK_UP) &1 && this->itemInfo.index > 0)
	{
		if (GetAsyncKeyState(VK_LSHIFT) < 0)
		{
			this->Update();
		}
		else
		{
			if (this->itemInfo.index < this->scrollInfo.index + 1)
				this->scrollInfo.index--;

			this->itemInfo.index--;
		}
	}
	if (GetAsyncKeyState(VK_DOWN) &1 && this->itemInfo.index < this->NumTabItems() - 1)
	{
		if (GetAsyncKeyState(VK_LSHIFT) < 0)
		{
			this->itemInfo.index = this->NumTabItems() - 1;

			if (this->NumScrollItems() > 0)
				this->scrollInfo.index = this->NumScrollItems();
		}
		else
		{
			if (this->itemInfo.index > this->scrollInfo.index + (this->scrollInfo.num - 2))
				this->scrollInfo.index++;

			this->itemInfo.index++;
		}
	}
	if (GetAsyncKeyState(VK_LEFT) &1)
	{
		if (GetAsyncKeyState(VK_LSHIFT) < 0)
		{
			if (this->tabInfo.index > 0)
			{
				this->tabInfo.index--;
				Update();
			}
		}
		else if (GetAsyncKeyState(VK_CONTROL) < 0)
		{
			if (this->colorInfo.index > 0)
				this->colorInfo.index--;
		}
		else
		{
			if (this->items[index].value > 0 && this->itemInfo.index >= 0 )
				this->items[index].value--;
		}
	}
	if (GetAsyncKeyState(VK_RIGHT) &1)
	{
		if (GetAsyncKeyState(VK_LSHIFT) < 0)
		{
			if (this->tabInfo.index < this->tabInfo.num - 1)
			{
				this->tabInfo.index++;
				this->Update();
			}
		}
		else if (GetAsyncKeyState(VK_CONTROL) < 0)
		{
			if (this->colorInfo.index < this->colorInfo.num - 1)
				this->colorInfo.index++;
		}
		else if (this->items[index].value < this->items[index].max && this->itemInfo.index >= 0)
			this->items[index].value++;
	}
}

void Menu::DrawSelection(int x, int y, int w)
{
	if (this->itemInfo.index >= 0)
	{
		draw.Rectangle(x, y, w, 14, D3DCOLOR_ARGB(255, 4, 4, 4));
		draw.Rectangle(x + 1, y + 1, w - 1, 13, SelectionColor);
	}
}

void Menu::DrawTabs(int x, int y, int w, int h)
{
	for (int i = 0; i < this->tabInfo.num; i++)
	{
		D3DCOLOR color = this->tabInfo.index == i ? this->menuInfo.color : OffColor;

		if (this->tabInfo.index == i)
			draw.Rectangle(x, y + i * h, w, h, EndColor, StartColor, BorderColor);
		else
			draw.Rectangle(x, y + i * h, w, h, StartColor, EndColor, BorderColor);

		if (this->IsMouseOver(x, y + (i * h), w, h) && this->tabInfo.index != i)
		{
			// highlight
			color += ColorOffset;

			if (this->IsLClicked())
			{
				this->tabInfo.index = i;
				this->Update();
			}
		}
		// centralizar horizontalmente									x + w/2
		// posicionar verticalmente de acordo com o índice o altura		(i * h)
		// centralizar verticalmente com o centro da aba				(h/2)
		// subtrair o a posição do topo do texto						-8
		draw.Text(x + w/2, y + (i * h) + (h/2) - 8, this->tabs[i].name, color, false, TextAlignment::kCenter);
	}
}

void Menu::MouseNavigation(int itemIndex)
{
	if (this->IsLClicked())
	{
		if (this->items[itemIndex].value < this->items[itemIndex].max)
			this->items[itemIndex].value++;
		else
			this->items[itemIndex].value = 0;
	}

	if (this->IsRClicked())
	{
		if (this->items[itemIndex].value > 0)
			this->items[itemIndex].value--;
		else
			this->items[itemIndex].value = this->items[itemIndex].max;
	}
}

void Menu::DrawItems(int x, int y, int w)
{
	int itemOffset = y;

	for (int i = this->FirstScrollIndex(); i <= this->LastScrollIndex(); i++)
	{
		D3DCOLOR color = this->items[i].value > 0 ? this->menuInfo.color : OffColor;

		// nome da função
		draw.Text(x, itemOffset + 5, this->items[i].name.c_str(), color);

		// lista de opções
		draw.Text(x + w - w/4 - 30, itemOffset + 5, this->items[i].display[this->items[i].value], color, false, TextAlignment::kRight);

		if (IsMouseOver(x - 5, itemOffset + 5, w - w/4 - 20, 16))
		{
			this->itemInfo.index = i - this->FirstScrollIndex() + this->scrollInfo.index;
			this->MouseNavigation(i);
		}
		// espaçamento de 16px para cada item no menu
		itemOffset += 16;
	}
}

void Menu::DrawColors(int x, int y, int w, int h)
{
	D3DCOLOR colors[] =
	{
		D3DCOLOR_ARGB(255, 255, 0, 70),		// vermelho
		D3DCOLOR_ARGB(255, 0, 120, 210),	// azul
		D3DCOLOR_ARGB(255, 0, 210, 70),		// verde água
		D3DCOLOR_ARGB(255, 255, 240, 0),	// amarelo
		D3DCOLOR_ARGB(255, 255, 120, 0),	// laranja
	};

	this->colorInfo.num = GetSizeOf(colors);

	// posicão x inicial relativo à largura total de todas as cores
	x += w - (this->colorInfo.num * 15); // crescente

	for (int i = 0; i < this->colorInfo.num; i++)
	{
		if (this->colorInfo.index == i)
		{
			draw.Rectangle(x + (i * 15) - 1, y + h - 1, 10, 10, colors[i], NULL, BorderColor);
		}
		else
		{
			if (this->IsMouseOver(x + (i * 15) - 1, y + h - 1, 10, 10))
			{
				if (this->IsLClicked())
					this->colorInfo.index =  i;

				draw.Rectangle(x + (i * 15) - 1, y + h - 1, 10, 10, colors[i], NULL, SelectionColor);
			}
			else
				draw.Rectangle(x + (i * 15), y + h, 8, 8, colors[i]);
		}
		this->menuInfo.color = colors[this->colorInfo.index];
	}
}

void Menu::DrawScrollBar(int x, int y, int h)
{
	// base do scroll
	draw.Rectangle(x, y, 1, h, BorderColor, NULL, BorderColor);

	// percentual de preenchimento de acordo com a quantidade de itens
	float fillPercentage = min((this->scrollInfo.num * 100)/this->NumTabItems(), 100);

	// tamanho do scroll é relativo ao percentual de preenchimento
	float scrollHeight = (fillPercentage * h)/100;

	// offset de acordo com o tamanho do scroll
	int scrollOffset = this->HasScroll() ? ((h - scrollHeight)/this->NumScrollItems()) : 0;

	// posição de acordo com o índice do scroll
	int indexOffset = this->scrollInfo.index * scrollOffset;

	// preencher de acordo com o índice e posição do scroll
	draw.Rectangle(x, y, 1, indexOffset - 1, StartColor, NULL, BorderColor);

	// desenhar scroll
	draw.Rectangle(x - 2, y + indexOffset, 6, scrollHeight, StartColor, EndColor, BorderColor);
}

void Menu::DragMenu(int x, int y, int w, int h)
{
	if (!this->IsHolding())
		this->menuInfo.isDragging = false;

	if (this->menuInfo.isDragging) {
		this->menuInfo.x = this->c.pos.x - this->c.offset.x;
		this->menuInfo.y = this->c.pos.y - this->c.offset.y;
	}

	if (this->IsMouseOver(x, y, w, h)) {
		this->c.offset.x = this->c.pos.x - this->menuInfo.x;
		this->c.offset.y = this->c.pos.y - this->menuInfo.y;
		this->menuInfo.isDragging = true;
	}
}
