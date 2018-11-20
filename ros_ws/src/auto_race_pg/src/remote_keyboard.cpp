#include <ros/ros.h>

#include <std_msgs/Float64.h>
#include <std_msgs/Int64.h>

#include <signal.h>
#include <termios.h>
#include <time.h>

#define MODE "keyboard"

#define TOPIC_SPEED "/set/speed"
#define TOPIC_ANGLE "/set/position"

#define TOPIC_STATUS_DMS "/status/dms"

#define KEYCODE_W 119
#define KEYCODE_A 97
#define KEYCODE_S 115
#define KEYCODE_D 100
#define KEYCODE_SPACE 32

#define DELTA_SPEED_UP 0.2
#define DELTA_SPEED_DOWN 0.1

#define DELTA_ANGLE_UP 0.2
#define DELTA_ANGLE_DOWN 0.1

class RemoteKeyboard
{
    public:
    RemoteKeyboard();
    void keyLoop();

    private:
    int getch();

    void adjustSpeed(double speed);
    void adjustAngle(double angle);

    ros::NodeHandle nh_;

    std::string input;

    ros::Publisher out_speed;
    ros::Publisher out_angle;
    ros::Publisher out_dms;

    double speed;
    double angle;
};

RemoteKeyboard::RemoteKeyboard()
    : speed(0)
    , angle(0)
{
    out_speed = nh_.advertise< std_msgs::Float64 >(TOPIC_SPEED, 1);
    out_angle = nh_.advertise< std_msgs::Float64 >(TOPIC_ANGLE, 1);
    out_dms   = nh_.advertise< std_msgs::Int64 >(TOPIC_STATUS_DMS, 1);
}

void RemoteKeyboard::keyLoop()
{
    std::cout << "listening to keyboard" << std::endl;
    std::cout << "=====================" << std::endl;

    while (ros::ok())
    {
        int  c           = getch();
        bool changeSpeed = false;
        bool changeAngle = false;

        if (c == KEYCODE_W)
        {
            speed       = speed + DELTA_SPEED_UP;
            changeSpeed = true;
        }

        if (c == KEYCODE_S)
        {
            speed       = speed - DELTA_SPEED_UP;
            changeSpeed = true;
        }

        speed = std::max(std::min(speed, 1.0), -1.0);

        if (!changeSpeed)
        {
            if (std::abs(speed) > DELTA_SPEED_DOWN)
            {
                speed = copysign(DELTA_SPEED_DOWN, -speed);
            }
            else
            {
                speed = 0;
            }
        }

        if (c == KEYCODE_A)
        {
            angle       = angle - DELTA_ANGLE_UP;
            changeAngle = true;
        }

        if (c == KEYCODE_D)
        {
            angle       = angle + DELTA_ANGLE_UP;
            changeAngle = true;
        }

        angle = std::max(std::min(angle, 1.0), -1.0);

        if (!changeAngle)
        {
            if (std::abs(angle) > DELTA_ANGLE_DOWN)
            {
                angle = copysign(DELTA_ANGLE_DOWN, -angle);
            }
            else
            {
                angle = 0;
            }
        }

        if (c == KEYCODE_SPACE)
        {

            std_msgs::Int64 msg;
            msg.data = (long)(ros::Time::now().toSec() * 1000);
            out_dms.publish(msg);
        }

        // after
        adjustSpeed(speed);
        adjustAngle(angle);
    }
}

int RemoteKeyboard::getch()
{
    static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); // save old settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON);               // disable buffering
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // apply new settings

    int c = getchar(); // read character (non-blocking)

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restore old settings
    return c;
}

void RemoteKeyboard::adjustSpeed(double speed)
{
    std_msgs::Float64 msg;
    msg.data = speed;
    out_speed.publish(msg);
}

void RemoteKeyboard::adjustAngle(double angle)
{
    std_msgs::Float64 msg;
    msg.data = (angle + 1) / 2;
    out_angle.publish(msg);
}

void quit(int sig)
{
    ros::shutdown();
    exit(0);
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "RemoteKeyboard");
    RemoteKeyboard remote_keyboard;

    std::cout << "listining to keyboard" << std::endl;
    std::cout << "=====================" << std::endl;

    signal(SIGINT, quit);
    remote_keyboard.keyLoop();
    return 0;
}