#ifndef TELNETTASK_MENUAPPLICATION_HPP
#define TELNETTASK_MENUAPPLICATION_HPP


#include "TCPSocket.hpp"
#include "RemoteTerminal.hpp"



class Menu
{
public:
    virtual unsigned int countItems() const = 0;
    virtual std::string getItem(unsigned int item) const = 0;
    virtual void triggerItem(unsigned int item, class MenuApplication &sender) const = 0;
};


class MainMenu : public Menu
{
public:
    unsigned int countItems() const override;
    std::string getItem(unsigned int item) const override;
    void triggerItem(unsigned int item, struct MenuApplication &sender) const override;
};


class SubMenu : public Menu
{
public:
    unsigned int countItems() const override;
    std::string getItem(unsigned int item) const override;
    void triggerItem(unsigned int item, struct MenuApplication &sender) const override;
};



class MenuApplication
{
private:
    RemoteTerminal terminal_;
    Menu *activeMenu_ = nullptr;
    int activeItem_ = 0;

    bool quit_ = false;
    unsigned int baseX_, baseY_;
    unsigned int requiredWidth_, requiredHeight_;

    void drawMenu_();


public:
    explicit MenuApplication(TCPCharStream &&stream);
    MenuApplication(const MenuApplication &other) = delete;
    MenuApplication(MenuApplication &&other) = delete;
    ~MenuApplication();

    void run();

    void output(std::string);
    void selectMenu(unsigned int menu_id);
    void quit();

    void listenKeys();
};



#endif //TELNETTASK_MENUAPPLICATION_HPP
