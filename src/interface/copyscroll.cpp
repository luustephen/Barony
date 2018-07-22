/*-------------------------------------------------------------------------------

BARONY
File: copyscroll.cpp
Desc: GUI code for the copy scroll spell.

Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "../main.hpp"
#include "../draw.hpp"
#include "../game.hpp"
#include "../stat.hpp"
#include "../items.hpp"
#include "../net.hpp"
#include "../player.hpp"
#include "interface.hpp"


bool itemIsScroll(Item* item);

bool itemIsScroll(Item* item)
{
	ItemType scrolls[] = {
		SCROLL_MAIL,
		SCROLL_IDENTIFY,
		SCROLL_LIGHT,
		SCROLL_ENCHANTWEAPON,
		SCROLL_ENCHANTARMOR,
		SCROLL_REMOVECURSE,
		SCROLL_FIRE,
		SCROLL_FOOD,
		SCROLL_MAGICMAPPING,
		SCROLL_REPAIR,
		SCROLL_DESTROYARMOR,
		SCROLL_TELEPORTATION,
		SCROLL_SUMMON,
		SCROLL_EQUIPMENT,
		SCROLL_WAR
	}; // Purposefully leave out blank scroll
	for (ItemType i : scrolls)
	{
		if (item->type == i)
		{
			return true;
		}
	}
	return false;
}



//Remove curse GUI definitions.
bool copyscrollgui_active = false;
bool copyscrollgui_appraising = false;
int copyscrollgui_offset_x = 0;
int copyscrollgui_offset_y = 0;
bool dragging_copyscrollGUI = false;
int copyscroll = 0;
Item* copyscroll_items[NUM_COPY_SCROLL_GUI_ITEMS];
SDL_Surface* copyscrollGUI_img;

int selectedCopyScrollSlot = -1;



void rebuildCopyScrollGUIInventory()
{
	list_t* copyscroll_inventory = &stats[clientnum]->inventory;
	node_t* node = nullptr;
	Item* item = nullptr;
	int c = 0;

	if (copyscroll_inventory)
	{
		//Count the number of items in the Remove Curse GUI "inventory".
		for (node = copyscroll_inventory->first; node != nullptr; node = node->next)
		{
			item = (Item*)node->element;
			if (item && item->identified && item->beatitude < 0)
			{
				++c;
			}
		}
		copyscroll = std::max(0, std::min(copyscroll, c - 4));
		for (c = 0; c < 4; ++c)
		{
			copyscroll_items[c] = nullptr;
		}
		c = 0;

		//Assign the visible items to the GUI slots.
		for (node = copyscroll_inventory->first; node != nullptr; node = node->next)
		{
			if (node->element)
			{
				item = (Item*)node->element;
				if (item && item->identified && itemIsScroll(item)) //Skip over all unidentified or uncursed items.
				{
					++c;
					if (c <= copyscroll)
					{
						continue;
					}
					copyscroll_items[c - copyscroll - 1] = item;
					if (c > 3 + copyscroll)
					{
						break;
					}
				}
			}
		}
	}
}


void updateCopyScrollGUI(int player)
{
	SDL_Rect pos;
	node_t* node;
	int y, c;
	//Remove Curse GUI.
	if (copyscrollgui_active)
	{
		//Center the remove curse GUI.
		pos.x = COPYSCROLL_GUI_X;
		pos.y = COPYSCROLL_GUI_Y;
		drawImage(identifyGUI_img, NULL, &pos);

		//Buttons
		if (mousestatus[SDL_BUTTON_LEFT])
		{
			//Remove Curse GUI scroll up button.
			if (omousey >= COPYSCROLL_GUI_Y + 16 && omousey < COPYSCROLL_GUI_Y + 52)
			{
				if (omousex >= COPYSCROLL_GUI_X + (identifyGUI_img->w - 28) && omousex < COPYSCROLL_GUI_X + (identifyGUI_img->w - 12))
				{
					buttonclick = 7;
					copyscroll--;
					mousestatus[SDL_BUTTON_LEFT] = 0;
				}
			}
			//Remove Curse GUI scroll down button.
			else if (omousey >= COPYSCROLL_GUI_Y + 52 && omousey < COPYSCROLL_GUI_Y + 88)
			{
				if (omousex >= COPYSCROLL_GUI_X + (identifyGUI_img->w - 28) && omousex < COPYSCROLL_GUI_X + (identifyGUI_img->w - 12))
				{
					buttonclick = 8;
					copyscroll++;
					mousestatus[SDL_BUTTON_LEFT] = 0;
				}
			}
			else if (omousey >= COPYSCROLL_GUI_Y && omousey < COPYSCROLL_GUI_Y + 15)
			{
				//Remove Curse GUI close button.
				if (omousex >= COPYSCROLL_GUI_X + 393 && omousex < COPYSCROLL_GUI_X + 407)
				{
					buttonclick = 9;
					mousestatus[SDL_BUTTON_LEFT] = 0;
				}
				if (omousex >= COPYSCROLL_GUI_X && omousex < COPYSCROLL_GUI_X + 377 && omousey >= COPYSCROLL_GUI_Y && omousey < COPYSCROLL_GUI_Y + 15)
				{
					gui_clickdrag = true;
					dragging_copyscrollGUI = true;
					dragoffset_x = omousex - COPYSCROLL_GUI_X;
					dragoffset_y = omousey - COPYSCROLL_GUI_Y;
					mousestatus[SDL_BUTTON_LEFT] = 0;
				}
			}
		}

		// mousewheel
		if (omousex >= COPYSCROLL_GUI_X + 12 && omousex < COPYSCROLL_GUI_X + (identifyGUI_img->w - 28))
		{
			if (omousey >= COPYSCROLL_GUI_Y + 16 && omousey < COPYSCROLL_GUI_Y + (identifyGUI_img->h - 8))
			{
				if (mousestatus[SDL_BUTTON_WHEELDOWN])
				{
					mousestatus[SDL_BUTTON_WHEELDOWN] = 0;
					copyscroll++;
				}
				else if (mousestatus[SDL_BUTTON_WHEELUP])
				{
					mousestatus[SDL_BUTTON_WHEELUP] = 0;
					copyscroll--;
				}
			}
		}

		if (dragging_copyscrollGUI)
		{
			if (gui_clickdrag)
			{
				copyscrollgui_offset_x = (omousex - dragoffset_x) - (COPYSCROLL_GUI_X - copyscrollgui_offset_x);
				copyscrollgui_offset_y = (omousey - dragoffset_y) - (COPYSCROLL_GUI_Y - copyscrollgui_offset_y);
				if (COPYSCROLL_GUI_X <= camera.winx)
				{
					copyscrollgui_offset_x = camera.winx - (COPYSCROLL_GUI_X - copyscrollgui_offset_x);
				}
				if (COPYSCROLL_GUI_X > camera.winx + camera.winw - identifyGUI_img->w)
				{
					copyscrollgui_offset_x = (camera.winx + camera.winw - identifyGUI_img->w) - (COPYSCROLL_GUI_X - copyscrollgui_offset_x);
				}
				if (COPYSCROLL_GUI_Y <= camera.winy)
				{
					copyscrollgui_offset_y = camera.winy - (COPYSCROLL_GUI_Y - copyscrollgui_offset_y);
				}
				if (COPYSCROLL_GUI_Y > camera.winy + camera.winh - identifyGUI_img->h)
				{
					copyscrollgui_offset_y = (camera.winy + camera.winh - identifyGUI_img->h) - (COPYSCROLL_GUI_Y - copyscrollgui_offset_y);
				}
			}
			else
			{
				dragging_copyscrollGUI = false;
			}
		}

		list_t* copyscroll_inventory = &stats[clientnum]->inventory;

		if (!copyscroll_inventory)
		{
			messagePlayer(0, "Warning: stats[%d].inventory is not a valid list. This should not happen.", clientnum);
		}
		else
		{
			//Print the window label signifying this as the remove curse GUI.
			char* window_name;
			window_name = language[4104];
			ttfPrintText(ttf8, (COPYSCROLL_GUI_X + 2 + ((identifyGUI_img->w / 2) - ((TTF8_WIDTH * longestline(window_name)) / 2))), COPYSCROLL_GUI_Y + 4, window_name);

			//Remove Curse GUI up button.
			if (buttonclick == 7)
			{
				pos.x = COPYSCROLL_GUI_X + (identifyGUI_img->w - 28);
				pos.y = COPYSCROLL_GUI_Y + 16;
				pos.w = 0;
				pos.h = 0;
				drawImage(invup_bmp, NULL, &pos);
			}
			//Remove Curse GUI down button.
			if (buttonclick == 8)
			{
				pos.x = COPYSCROLL_GUI_X + (identifyGUI_img->w - 28);
				pos.y = COPYSCROLL_GUI_Y + 52;
				pos.w = 0;
				pos.h = 0;
				drawImage(invdown_bmp, NULL, &pos);
			}
			//Remove Curse GUI close button.
			if (buttonclick == 9)
			{
				pos.x = COPYSCROLL_GUI_X + 393;
				pos.y = COPYSCROLL_GUI_Y;
				pos.w = 0;
				pos.h = 0;
				drawImage(invclose_bmp, NULL, &pos);
				closeCopyScrollGUI();
			}

			Item *item = nullptr;

			bool selectingSlot = false;
			SDL_Rect slotPos;
			slotPos.x = COPYSCROLL_GUI_X;
			slotPos.w = inventoryoptionChest_bmp->w;
			slotPos.y = COPYSCROLL_GUI_Y + 16;
			slotPos.h = inventoryoptionChest_bmp->h;

			for (int i = 0; i < NUM_COPY_SCROLL_GUI_ITEMS; ++i, slotPos.y += slotPos.h)
			{
				pos.x = slotPos.x + 12;
				pos.w = 0;
				pos.h = 0;

				if (omousey >= slotPos.y && omousey < slotPos.y + slotPos.h && copyscroll_items[i])
				{
					pos.y = slotPos.y;
					drawImage(inventoryoptionChest_bmp, nullptr, &pos);
					selectedCopyScrollSlot = i;
					selectingSlot = true;
					if (mousestatus[SDL_BUTTON_LEFT] || *inputPressed(joyimpulses[INJOY_MENU_USE]))
					{
						*inputPressed(joyimpulses[INJOY_MENU_USE]) = 0;
						mousestatus[SDL_BUTTON_LEFT] = 0;
						copyscrollGUICopyScroll(copyscroll_items[i], player);

						rebuildCopyScrollGUIInventory();
						if (copyscroll_items[i] == nullptr)
						{
							if (copyscroll_items[0] == nullptr)
							{
								//Go back to inventory.
								selectedCopyScrollSlot = -1;
								warpMouseToSelectedInventorySlot();
							}
							else
							{
								//Move up one slot.
								--selectedCopyScrollSlot;
								warpMouseToSelectedCopyScrollSlot();
							}
						}
					}
				}
			}

			if (!selectingSlot)
			{
				selectedCopyScrollSlot = -1;
			}

			//Okay, now prepare to render all the items.
			y = COPYSCROLL_GUI_Y + 22;
			c = 0;
			if (copyscroll_inventory)
			{
				rebuildCopyScrollGUIInventory();

				//Actually render the items.
				c = 0;
				for (node = copyscroll_inventory->first; node != NULL; node = node->next)
				{
					if (node->element)
					{
						item = (Item*)node->element;
						if (item && item->identified && itemIsScroll(item))   //Skip over all unidentified or uncursed items.
						{
							c++;
							if (c <= copyscroll)
							{
								continue;
							}
							char tempstr[64] = { 0 };
							strncpy(tempstr, item->description(), 46);
							if (strlen(tempstr) == 46)
							{
								strcat(tempstr, " ...");
							}
							ttfPrintText(ttf8, COPYSCROLL_GUI_X + 36, y, tempstr);
							pos.x = COPYSCROLL_GUI_X + 16;
							pos.y = COPYSCROLL_GUI_Y + 17 + 18 * (c - copyscroll - 1);
							pos.w = 16;
							pos.h = 16;
							drawImageScaled(itemSprite(item), NULL, &pos);
							y += 18;
							if (c > 3 + copyscroll)
							{
								break;
							}
						}
					}
				}
			}
		}
	}
} //updateCopyScrollGUI()

void copyscrollGUICopyScroll(Item* item, int player)
{
	if (!item)
	{
		return;
	}

	newItem(item->type, item->status, item->beatitude, 1, rand(), true, &stats[clientnum]->inventory);
	list_t* copyInv = &stats[clientnum]->inventory;
	node_t* node = nullptr;
	item = nullptr;
	for (node = copyInv->first; node != nullptr; node = node->next)
	{
		item = (Item*)node->element;
		if (item->type == SCROLL_BLANK)
		{
			consumeItem(item);
			break;
		}
	}
	messagePlayer(clientnum, language[4105]);
	closeCopyScrollGUI();
}

void closeCopyScrollGUI()
{
	copyscrollgui_active = false;

	selectedCopyScrollSlot = -1;
}

inline Item* getItemInfoFromCopyScrollGUI(int slot)
{
	if (slot >= 4)
	{
		return nullptr; //Out of bounds,
	}

	return copyscroll_items[slot];
}

void selectCopyScrollSlot(int slot)
{
	if (slot < selectedCopyScrollSlot)
	{
		//Moving up.

		/*
		* Possible cases:
		* * 1) Move cursor up the GUI through different selectedCopyScrollSlot.
		* * 2) Page up through copyscroll--
		* * 3) Scrolling up past top of Remove Curse GUI, no copyscroll (move back to inventory)
		*/

		if (selectedCopyScrollSlot <= 0)
		{
			//Covers cases 2 & 3.

			/*
			* Possible cases:
			* * A) Hit very top of Remove Curse "inventory", can't go any further. Return to inventory.
			* * B) Page up, scrolling through copyscroll.
			*/

			if (copyscroll <= 0)
			{
				//Case 3/A: Return to inventory.
				selectedCopyScrollSlot = -1;
			}
			else
			{
				//Case 2/B: Page up through Remove Curse "inventory".
				--copyscroll;
			}
		}
		else
		{
			//Covers case 1.

			//Move cursor up the GUI through different selectedCopyScrollSlot (--selectedCopyScrollSlot).
			--selectedCopyScrollSlot;
			warpMouseToSelectedCopyScrollSlot();
		}
	}
	else if (slot > selectedCopyScrollSlot)
	{
		//Moving down.

		/*
		* Possible cases:
		* * 1) Moving cursor down through GUI through different selectedCopyScrollSlot.
		* * 2) Scrolling down past bottom of Remove Curse GUI through copyscroll++
		* * 3) Scrolling down past bottom of Remove Curse GUI, max Remove Curse scroll (revoke move -- can't go beyond limit of Remove Curse GUI).
		*/

		if (selectedCopyScrollSlot >= NUM_COPY_SCROLL_GUI_ITEMS - 1)
		{
			//Covers cases 2 & 3.
			++copyscroll; //copyscroll is automatically sanitized in updateCopyScrollGUI().
		}
		else
		{
			//Covers case 1.
			//Move cursor down through the GUI through different selectedCopyScrollSlot (++selectedCopyScrollSlot).
			//This is a little bit trickier since must revoke movement if there is no item in the next slot!

			/*
			* Two possible cases:
			* * A) Items below this. Advance selectedCopyScrollSlot to them.
			* * B) On last item already. Do nothing (revoke movement).
			*/

			Item* item = getItemInfoFromCopyScrollGUI(selectedCopyScrollSlot + 1);

			if (item)
			{
				++selectedCopyScrollSlot;
				warpMouseToSelectedCopyScrollSlot();
			}
			else
			{
				//No more items. Stop.
			}
		}
	}
}

void warpMouseToSelectedCopyScrollSlot()
{
	SDL_Rect slotPos;
	slotPos.x = COPYSCROLL_GUI_X;
	slotPos.w = inventoryoptionChest_bmp->w;
	slotPos.h = inventoryoptionChest_bmp->h;
	slotPos.y = COPYSCROLL_GUI_Y + 16 + (slotPos.h * selectedCopyScrollSlot);

	SDL_WarpMouseInWindow(screen, slotPos.x + (slotPos.w / 2), slotPos.y + (slotPos.h / 2));
}
