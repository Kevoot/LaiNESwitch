#include <dirent.h>
#include <unistd.h>
#include "cartridge.hpp"
#include "menu.hpp"
#include "config.hpp"
#include <switch.h>

namespace GUI {

using namespace std;


Entry::Entry(string label, function<void()> callback, int x, int y) : callback(callback), x(x), y(y)
{
    setLabel(label);
}

Entry::~Entry()
{
    SDL_DestroyTexture(whiteTexture);
    SDL_DestroyTexture(redTexture);
}

void Entry::setLabel(string label)
{
    this->label = label;

    if (whiteTexture != nullptr) SDL_DestroyTexture(whiteTexture);
    if (redTexture   != nullptr) SDL_DestroyTexture(redTexture);

    whiteTexture = gen_text(label, { 255, 255, 255 });
    redTexture   = gen_text(label, { 255,   0,   0 });
}

void Entry::render()
{
    render_texture(selected ? redTexture : whiteTexture, getX(), getY());
}


ControlEntry::ControlEntry(string action, SDL_Scancode* key, int x, int y) : key(key),
    Entry::Entry(
        action,
        [&]{ keyEntry->setLabel(SDL_GetScancodeName(*(this->key) = query_key())); },
        x,
        y)
{
    this->keyEntry = new Entry(SDL_GetScancodeName(*key), []{}, TEXT_RIGHT, y);
}

ControlEntry::ControlEntry(string action, int* button, int x, int y) : button(button),
    Entry::Entry(
        action,
        [&]{ keyEntry->setLabel(to_string(*(this->button) = query_button())); },
        x,
        y)
{
    this->keyEntry = new Entry(to_string(*button), []{}, TEXT_RIGHT, y);
}


void Menu::add(Entry* entry)
{
    if (entries.empty())
        entry->select();
    
    entries.push_back(entry);
}

void Menu::clear()
{
    for (auto entry : entries)
        delete entry;
    entries.clear();
    cursor = 0;
}

void Menu::update(int joy)
{
    int oldCursor = cursor;

    if (((joy == JOY_DOWN) or (joy == JOY_LSTICK_DOWN)) and cursor < entries.size())
        cursor++;
    else if (((joy == JOY_UP) or (joy == JOY_LSTICK_UP)) and cursor >= 0)
        cursor--;

	if (cursor < 0)
		cursor = entries.size()-1;
	else if (cursor >= entries.size())
		cursor = 0;

	displayOffset = cursor / entryDisplayLimit;

    entries[oldCursor]->unselect();
    entries[cursor]->select();

    if (joy == JOY_A)
        entries[cursor]->trigger();
}

void Menu::render()
{
   /* for (auto entry : entries)
        entry->render();*/
	for (int it = 0; it < entryDisplayLimit; it++) {
		int relPos = it;
		relPos += displayOffset * entryDisplayLimit;
		entries[relPos]->setY(it * FONT_SZ);
		entries[relPos]->render();
	}
}

void FileMenu::change_dir(string dir)
{
    clear();

    struct dirent* dirp;
    DIR* dp = opendir(dir.c_str());

    if(this->depth > 0) {
        add(new Entry("Back",
                            [=]{ this->depth = 0; change_dir(".."); },
                        0));
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        string name = dirp->d_name;
        string path = dir + "/" + name;

        if (name[0] == '.' and name != "..") continue;

        if (dirp->d_type == DT_DIR)
        {
            add(new Entry(name + "/",
                          [=]{ this->depth++; change_dir(path); },
                          0));
        }
        else if (name.size() > 4 and name.substr(name.size() - 4) == ".nes")
        {
            add(new Entry(name,
                          [=]{ 
                                if(Cartridge::load(path.c_str()) != -1){
                                    printf("Successfully loaded ROM\n");
                                    toggle_pause();
                                }
                                else {
                                    printf("Cartridge load failed\n");
                                    closedir(dp);
                                }
                            },
                          0));
        }
    }
    closedir(dp);
    return;
}

FileMenu::FileMenu()
{
    char cwd[512];
    this->depth = 0;
    change_dir(getcwd(cwd, 512));
}


}
