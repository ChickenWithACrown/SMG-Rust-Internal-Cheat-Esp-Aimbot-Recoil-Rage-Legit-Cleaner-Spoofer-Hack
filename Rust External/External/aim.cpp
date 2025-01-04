#include "aim.h"
#include <random>

// Checks if the target is within the Field of View (FOV)
bool InFov(BasePlayer& BasePlayer_on_Aimming, BoneList bone)
{
    Vector2 ScreenPos;
    if (!myLocalPlayer.WorldToScreen(BasePlayer_on_Aimming.GetBonePosition(head), &ScreenPos)) 
        return false;

    // Calculate the distance from the center of the screen
    return Math::Calc2D_Dist(Vector2(Vars::Config::ScreenWidth / 2, Vars::Config::ScreenHigh / 2), ScreenPos) <= Vars::Aim::fov;
}

// Normalize the angles to prevent exceeding valid ranges
void Normalize(Vector2& angle)
{
    angle.x = std::clamp(angle.x, -89.0f, 89.0f);  // Limit the X angle between -89 and 89
    angle.y = std::fmod(angle.y, 360.0f);  // Wrap Y angle between -180 and 180
    if (angle.y > 180) angle.y -= 360;  // Adjust Y angle to fit the -180 to 180 range
}

// Retrieves the bullet speed based on the active weapon
float GetBulletSpeed()
{
    switch (myLocalPlayer.myActiveWeapon.GetID())
    {
        case 1545779598: // AK47
        case 2482412119: // LR300
        case 3390104151: // Semi-rifle
            return 375.f;
        case 28201841: // M39
            return 375.f * 1.16f;
        case 2225388408: // M249
            return 375.f * 1.4f;
        case 1588298435: // Bolt-action
            return 375.f * 1.8f;
        case 3516600001: // L96
            return 375.f * 3.2f;
        case 1318558775: // MP5A4
        case 1796682209: // SMG
        case 2536594571: // Thompson
        case 3442404277: // M92
        case 818877484:  // Semi-pistol
        case 1373971859: // Python
        case 649912614:  // Revolver
            return 300.f;
        default:
            return 0.f;  // Unknown weapon, return 0
    }
}

// Predict the target's future position
Vector3 Prediction(const Vector3& my_Pos, BasePlayer& BasePlayer_on_Aimming, BoneList Bone)
{
    Vector3 BonePos = BasePlayer_on_Aimming.GetBonePosition(Bone);
    float Dist = Math::Calc3D_Dist(my_Pos, BonePos);

    float BulletSpeed = GetBulletSpeed();
    if (BulletSpeed <= 0) return BonePos;

    float BulletTime = Dist / BulletSpeed;
    Vector3 vel = BasePlayer_on_Aimming.GetVelocity();
    Vector3 Predict = vel * BulletTime;
    BonePos += Predict;

    // Adjust for bullet trajectory and angle
    float DegAngle = myLocalPlayer.GetBA().x;
    float coef = (DegAngle >= 10 && Dist <= 100) ? cos(DEG2RAD(DegAngle)) * 0.1f : cos(DEG2RAD(DegAngle)) * 0.9f;
    BonePos.y += (6.4f * BulletTime * BulletTime) * coef;  // Adjust for bullet drop over time

    return BonePos;
}

// Adjust the aim to go towards the target
void GoToTarget(BasePlayer& BasePlayer_on_Aimming, BoneList bone)
{
    Vector3 Local = myLocalPlayer.GetBonePosition(head);
    Vector3 PlayerPos = Prediction(Local, BasePlayer_on_Aimming, bone);
    Vector2 Offset = Math::CalcAngle(Local, PlayerPos) - myLocalPlayer.GetBA();

    Normalize(Offset);

    // Apply smoothing based on configuration
    Offset.x *= 1.0f - (Vars::Aim::smooth * 0.3f + 0.4f);
    Offset.y *= 1.0f - (Vars::Aim::smooth * 0.3f + 0.4f);

    Vector2 AngleToAim = myLocalPlayer.GetBA() + Offset;
    myLocalPlayer.SetBA(AngleToAim);
}

// Main function to handle aiming logic
void Aim(DWORD64& BasePlayer_on_Aimming)
{
    static BasePlayer Player;
    Player.set_addr(BasePlayer_on_Aimming);

    if (BasePlayer_on_Aimming && (GetAsyncKeyState(VK_RBUTTON) & 0x8000))
    {
        if (Player.IsDead()) 
            BasePlayer_on_Aimming = NULL;
        else
        {
            // Random bone selection if enabled
            static int boneArr[6] = { head, spine1, r_upperarm, l_breast, r_hand };
            static BoneList bone;
            static DWORD64 isBasePlayerChange = NULL;

            if (isBasePlayerChange != Player.get_addr())
            {
                // Use random bone selection logic
                static std::random_device rd;
                static std::mt19937 gen(rd());
                static std::uniform_int_distribution<int> dis(0, 5);
                bone = Vars::Aim::randomBone ? BoneList(boneArr[dis(gen)]) : head;
                isBasePlayerChange = Player.get_addr();
            }

            static int start = 0;
            static int summ = 0;
            summ += clock() - start;
            start = clock();

            if (summ >= 20)  // Refresh every 20ms
            {
                GoToTarget(Player, bone);
                summ = 0;
            }

            // Recoil Control System (RCS) logic
            static int RCSstart = 0;
            static int RCSsumm = 0;
            RCSsumm += clock() - RCSstart;
            RCSstart = clock();

            if (RCSsumm >= (0.2f + Vars::Aim::smooth * 0.3f) * 10)
            {
                myLocalPlayer.SetRA({ 0.f, 0.f });  // Reset recoil aim
                RCSsumm = 0;
            }
        }
    }
    else
    {
        BasePlayer_on_Aimming = NULL;  // Reset if not aiming
    }
}
