#ifndef  OCTOLCD_ACTIVITY
#define OCTOLCD_ACTIVITY

class Activity
{
public:
    virtual void childActivityHidden(Activity *child) = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
};

#endif
