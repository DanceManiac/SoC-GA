#pragma once

#include "motion.h"

// refs
class ENGINE_API CObjectAnimator
{
private:
    using MotionVec = xr_vector<COMotion*>;

protected:

    shared_str m_Name;

    Fmatrix m_XFORM;
    SAnimParams m_MParam;
    MotionVec m_Motions;
    float m_Speed;

public:
    bool bLoop;
    COMotion* m_Current;
    void LoadMotions(LPCSTR fname);
    void SetActiveMotion(COMotion* mot);
    COMotion* FindMotionByName(LPCSTR name);

    CObjectAnimator();
    virtual ~CObjectAnimator();

    void Clear();
    void Load(LPCSTR name);
    IC LPCSTR Name() { return *m_Name; }
    float& Speed() { return m_Speed; }

    COMotion* Play(bool bLoop, LPCSTR name = nullptr);
    void Pause(bool val) { return m_MParam.Pause(val); }
    void Stop();
    IC BOOL IsPlaying() { return m_MParam.bPlay; }
	const SAnimParams&	anim_param() { return m_MParam; }

    IC const Fmatrix& XFORM() { return m_XFORM; }
    float GetLength();
    // Update
    void Update(float dt);
    void DrawPath();
};
