#include <algorithm>
#include <thread>
#include "MenuApplication.hpp"



MenuApplication::MenuApplication(TCPCharStream &&stream):
    terminal_(std::move(stream))
{
    activeMenu_ = new MainMenu;
}


MenuApplication::~MenuApplication()
{
    delete activeMenu_;
}


void MenuApplication::run()
{
    do {
        drawMenu_();
        listenKeys();
    } while (!quit_);
}


void MenuApplication::output(std::string out)
{
    terminal_.move(baseX_, baseY_ + requiredHeight_ - 1);
    terminal_ << "\033[2K" << out;
    terminal_.flush();
}


void MenuApplication::selectMenu(unsigned int menu_id)
{
    Menu *new_menu = nullptr;
    switch (menu_id) {
        case 0:
            new_menu = new MainMenu;
            break;
        case 1:
            new_menu = new SubMenu;
        default:
            break;
    }
    if (new_menu == nullptr)
        return;
    activeItem_ = 0;
    activeMenu_ = new_menu;
}


void MenuApplication::quit()
{
    quit_ = true;
}


void MenuApplication::drawMenu_()
{
    requiredWidth_ = 4;
    requiredHeight_ = activeMenu_->countItems() + 1;
    for (unsigned int i = 0; i < activeMenu_->countItems(); ++i)
        requiredWidth_ = std::max(requiredWidth_, static_cast<unsigned int>(activeMenu_->getItem(i).length()) + 4);

    baseX_ = 0;
    baseY_ = 0;
    if (terminal_.hasNaws()) {
        baseX_ = (terminal_.screenWidth() - requiredWidth_) / 2;
        baseY_ = (terminal_.screenHeight() - requiredHeight_) / 2;
    }

    for (unsigned int i = 0; i < activeMenu_->countItems(); ++i) {
        const auto &item = activeMenu_->getItem(i);
        auto fill_w = (requiredWidth_ - 4 - item.length()) / 2;
        std::string border_l = (i == activeItem_ ? "[[" : "[ ");
        std::string border_r = (i == activeItem_ ? "]]" : " ]");

        terminal_.move(baseX_, baseY_ + i);
        terminal_ << "\033[2K" << border_l << std::string(fill_w, ' ') << item
                  << std::string(requiredWidth_ - 4 - fill_w - item.length(), ' ') << border_r;
    }
    terminal_.flush();
}


void MenuApplication::listenKeys()
{
    char buf[3];
    buf[1] = terminal_.get();
    buf[2] = terminal_.get();
    if (buf[1] == 13 && buf[2] == 0) {
        activeMenu_->triggerItem(static_cast<unsigned int>(activeItem_), *this);
        return;
    }

    do {
        buf[0] = buf[1];
        buf[1] = buf[2];
        buf[2] = terminal_.get();
        if (buf[1] == 13 && buf[2] == 0) {
            activeMenu_->triggerItem(static_cast<unsigned int>(activeItem_), *this);
            return;
        }
        if (buf[0] != 27 || buf[1] != '[')
            continue;

        if (buf[2] == 'A') {
            --activeItem_;
            break;
        }
        if (buf[2] == 'B') {
            ++activeItem_;
            break;
        }
    } while (true);

    activeItem_ = std::min(activeMenu_->countItems() - 1, static_cast<unsigned int>(std::max(0, activeItem_)));
}


unsigned int MainMenu::countItems() const
{
    return 3;
}


std::string MainMenu::getItem(unsigned int item) const
{
    switch (item) {
        case 0:
            return "Opcja A";
        case 1:
            return "Opcja B";
        case 2:
            return "Koniec";
        default:
            throw std::out_of_range("menu item index out of range");
    }
}


void MainMenu::triggerItem(unsigned int item, struct MenuApplication &sender) const
{
    switch (item) {
        case 0:
            sender.output("A");
            break;
        case 1:
            sender.selectMenu(1);
            break;
        case 2:
            sender.quit();
            break;
    }
}



unsigned int SubMenu::countItems() const
{
    return 3;
}


std::string SubMenu::getItem(unsigned int item) const
{
    switch (item) {
        case 0:
            return "Opcja B1";
        case 1:
            return "Opcja B2";
        case 2:
            return "Wstecz";
        default:
            throw std::out_of_range("menu item index out of range");
    }
}


void SubMenu::triggerItem(unsigned int item, struct MenuApplication &sender) const
{
    switch (item) {
        case 0:
            sender.output("B1");
            break;
        case 1:
            sender.output("B2");
            break;
        case 2:
            sender.selectMenu(0);
            break;
    }
}
