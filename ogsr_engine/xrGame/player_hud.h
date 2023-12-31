#pragma once
#include "firedeps.h"

#include "../Include/xrRender/Kinematics.h"
#include "../Include/xrRender/KinematicsAnimated.h"
#include "actor_defs.h"
#include "../xr_3da/ObjectAnimator.h"

class player_hud;
class CHudItem;
class CMotionDef;
struct attachable_hud_item;

struct motion_descr
{
    MotionID mid;
    shared_str name;
    float speed_k{1.0f};
    float stop_k{1.0f};
    const char* eff_name{};
};

struct player_hud_motion
{
    shared_str m_base_name;
    shared_str m_additional_name;
    xr_vector<motion_descr> m_animations;
};

struct player_hud_motion_container
{
    string_unordered_map<shared_str, player_hud_motion> m_anims;
    player_hud_motion* find_motion(const shared_str& name);
    void load(attachable_hud_item* parent, IKinematicsAnimated* model, IKinematicsAnimated* animatedHudItem, const shared_str& sect);
};

struct hud_item_measures
{
    enum
    {
        e_fire_point = (1 << 0),
        e_fire_point2 = (1 << 1),
        e_shell_point = (1 << 2),
        e_16x9_mode_now = (1 << 3)
    };
    Flags8 m_prop_flags;

    Fvector m_item_attach[2]{}; // pos,rot

    enum m_hands_offset_coords : u8
    {
        m_hands_offset_pos,
        m_hands_offset_rot,
        m_hands_offset_size
    };
    enum m_hands_offset_type : u8
    {
        m_hands_offset_type_normal, // Не прицеливаемся
        m_hands_offset_type_aim, // Смотрим в механический прицел
        m_hands_offset_type_gl, // Смотрим в механический прицел в режиме ПГ
        m_hands_offset_type_aim_scope, // Смотрим в присоединяемый нетекстурный прицел (будь то 3д прицел или колиматор) если включен "use_scope_zoom"
        m_hands_offset_type_gl_scope, // Смотрим в присоединяемый нетекстурный прицел (будь то 3д прицел или колиматор) в режиме ПГ если включен "use_scope_grenade_zoom" - мне вот
                                      // щас не понятно зачем это надо, но это как-то используют.
        m_hands_offset_type_aim_gl_normal, // Смотрим в механический прицел если гранатомет присоединен
        m_hands_offset_type_gl_normal_scope, // Смотрим в присоединяемый нетекстурный прицел (будь то 3д прицел или колиматор) если включен "use_scope_zoom" и гранатомет
                                             // присоединен
        m_hands_offset_type_size
    };
    Fvector m_hands_offset[m_hands_offset_size][m_hands_offset_type_size]{};

    u16 m_fire_bone;
    Fvector m_fire_point_offset;
    u16 m_fire_bone2;
    Fvector m_fire_point2_offset;
    u16 m_shell_bone;
    Fvector m_shell_point_offset;
    Fvector m_shoot_point_offset{};

    Fvector m_hands_attach[2]{}; // pos,rot

    void load(const shared_str& sect_name, IKinematics* K);

    bool useCopFirePoint{};
};

struct attachable_hud_item
{
    player_hud* m_parent;
    CHudItem* m_parent_hud_item{};
    shared_str m_sect_name;
    shared_str m_visual_name;
    IKinematics* m_model{};
    u16 m_attach_place_idx{};
    hud_item_measures m_measures{};
    bool m_has_separated_hands{};

    // runtime positioning
    Fmatrix m_attach_offset;
    Fmatrix m_item_transform;

    player_hud_motion_container m_hand_motions;

    attachable_hud_item(player_hud* pparent) : m_parent(pparent), m_upd_firedeps_frame(u32(-1)) {}
    ~attachable_hud_item();

    void load(const shared_str& sect_name);
    void update(bool bForce);
    void setup_firedeps(firedeps& fd);
    void render();
    void render_item_ui();
    bool render_item_ui_query();
    bool need_renderable();
    void set_bone_visible(const shared_str& bone_name, BOOL bVisibility, BOOL bSilent = FALSE);
    void set_bone_visible(const xr_vector<shared_str>& bone_names, BOOL bVisibility, BOOL bSilent = FALSE);
    BOOL get_bone_visible(const shared_str& bone_name);
    bool has_bone(const shared_str& bone_name);
    void debug_draw_firedeps();

    // hands bind position
    Fvector& hands_attach_pos();
    Fvector& hands_attach_rot();

    // hands runtime offset
    Fvector& hands_offset_pos();
    Fvector& hands_offset_rot();

    // props
    u32 m_upd_firedeps_frame;
    void tune(const Ivector& values);
    u32 anim_play(const shared_str& anim_name, BOOL bMixIn, const CMotionDef*& md, u8& rnd, bool randomAnim);
};

struct hand_motions
{
	const char* section;
	player_hud_motion_container pm;
};

struct item_models
{
	const char* name;
	IKinematicsAnimated* model;
};

struct script_layer
{
	const char* m_name;
	CObjectAnimator* anm;
	float blend_amount;
	float m_power;
	bool active;
	Fmatrix blend;
	u8 m_part;

	script_layer(const char* name, const u8 part, const float speed = 1.f, const float power = 1.f, const bool looped = true)
	{
		m_name = name;
		m_part = part;
		m_power = power;
		blend.identity();
		anm = xr_new<CObjectAnimator>();
		anm->Load(name);
		anm->Play(looped);
		anm->Speed() = speed;
		blend_amount = 0.f;
		active = true;
	}

	bool IsPlaying()
	{
		return anm->IsPlaying();
	}

	void Stop(bool bForce)
	{
		if (bForce)
		{
			anm->Stop();
			blend_amount = 0.f;
			blend.identity();
		}

		active = false;
	}

	const Fmatrix& XFORM()
	{
		blend.set(anm->XFORM());
		blend.mul(blend_amount * m_power);
		blend.m[0][0] = 1.f;
		blend.m[1][1] = 1.f;
		blend.m[2][2] = 1.f;

		return blend;
	}
};

enum eMovementLayers : u8
{
	eAimWalk = 0,
	eAimCrouch,
	eCrouch,
	eWalk,
	eRun,
	eSprint,
	move_anms_end
};

struct movement_layer
{
	CObjectAnimator* const anm = xr_new<CObjectAnimator>();
	float blend_amount[2]{};
	bool active{};
	float m_power{};
	Fmatrix blend{};
	u8 m_part{};

	movement_layer()
	{
		blend.identity();
		blend_amount[0] = 0.f;
		blend_amount[1] = 0.f;
		active = false;
		m_power = 1.f;
	}

	void Load(const char* name)
	{
		if (xr_strcmp(name, anm->Name()))
			anm->Load(name);
	}

	void Play(const bool bLoop = true)
	{
		if (!anm->Name())
			return;

		if (IsPlaying())
		{
			active = true;
			return;
		}
		
		anm->Play(bLoop);
		active = true;
	}

	const bool IsPlaying() const
    {
		return anm->IsPlaying();
	}

	void Stop(const bool bForce)
	{
		if (bForce)
		{
			anm->Stop();
			blend_amount[0] = 0.f;
			blend_amount[1] = 0.f;
			blend.identity();
		}

		active = false;
	}

	const Fmatrix& XFORM(const u8 part)
	{
		blend.set(anm->XFORM());
		blend.mul(blend_amount[part] * m_power);
		blend.m[0][0] = 1.f;
		blend.m[1][1] = 1.f;
		blend.m[2][2] = 1.f;

		return blend;
	}
};

enum eSecondHandStates : u8
{
	eNoState,
	eTakeItem,
};

class player_hud
{
public:
    player_hud();
    ~player_hud();
    void load(const shared_str& model_name);
    void load_default() { load("actor_hud_05"); };
    void update(const Fmatrix& trans);
	void StopScriptAnim();
	void updateMovementLayerState();
	void PlayBlendAnm(const char* name, const u8 part = 0, const float speed = 1.f, const float power = 1.f, const bool bLooped = true, const bool no_restart = false);
	void StopBlendAnm(const char* name, const bool bForce = false);
	void StopAllBlendAnms(const bool bForce);
	float SetBlendAnmTime(const char* name, const float time);
	u32 script_anim_play(const u8 hand, const char* itm_name, const char* anm_name, const bool bMixIn = true, const float speed = 1.f);
	bool allow_script_anim();
	u32 motion_length_script(const char* section, const char* anm_name, const float speed);
    void render_hud();
    void render_item_ui();
    bool render_item_ui_query();
    u32 anim_play(u16 part, const motion_descr& M, BOOL bMixIn, const CMotionDef*& md, float speed, bool hasHands, IKinematicsAnimated* itemModel = nullptr,
                  u16 override_part = u16(-1));
    const shared_str& section_name() const { return m_sect_name; }
    attachable_hud_item* create_hud_item(const shared_str& sect);

    void attach_item(CHudItem* item);
    bool allow_activation(CHudItem* item);
    attachable_hud_item* attached_item(u16 item_idx) { return m_attached_items[item_idx]; };
    void detach_item_idx(u16 idx);
    void detach_item(CHudItem* item);
    void detach_all_items()
    {
        m_attached_items[0] = nullptr;
        m_attached_items[1] = nullptr;
    }

    void calc_transform(u16 attach_slot_idx, const Fmatrix& offset, Fmatrix& result);
    void tune(const Ivector& values);
	void SaveCfg(const int idx) const;

    u32 motion_length(const motion_descr& M, const CMotionDef*& md, float speed, bool hasHands, IKinematicsAnimated* itemModel, attachable_hud_item* pi = nullptr);
    u32 motion_length(const shared_str& anim_name, const shared_str& hud_name, const CMotionDef*& md);
    void OnMovementChanged(ACTOR_DEFS::EMoveCommand cmd);
    void re_sync_anim(u8 part);
    void GetLHandBoneOffsetPosDir(const shared_str& bone_name, Fvector& dest_pos, Fvector& dest_dir, const Fvector& offset);

	std::vector<movement_layer*> m_movement_layers;

    Fvector target_thumb0rot{}, target_thumb01rot{}, target_thumb02rot{};
    Fvector thumb0rot{}, thumb01rot{}, thumb02rot{};
    void reset_thumb(bool bForce)
    {
        if (bForce)
        {
            thumb0rot.set(0.f, 0.f, 0.f);
            thumb01rot.set(0.f, 0.f, 0.f);
            thumb02rot.set(0.f, 0.f, 0.f);
        }
        target_thumb0rot.set(0.f, 0.f, 0.f);
        target_thumb01rot.set(0.f, 0.f, 0.f);
        target_thumb02rot.set(0.f, 0.f, 0.f);
    }

	u8 script_anim_part;
private:
	Fvector script_anim_offset[2];
	u32 script_anim_end;
	float script_anim_offset_factor;
	bool m_bStopAtEndAnimIsRunning;
	bool script_anim_item_attached;
	IKinematicsAnimated* script_anim_item_model;
	Fvector item_pos[2];
	Fmatrix m_item_pos;
	u8 m_attach_idx;

	xr_vector<script_layer*> m_script_layers;
	xr_vector<hand_motions*> m_hand_motions;
	player_hud_motion_container* get_hand_motions(LPCSTR section);

	void update_script_item();

    static void Thumb0Callback(CBoneInstance* B);
    static void Thumb01Callback(CBoneInstance* B);
    static void Thumb02Callback(CBoneInstance* B);

    shared_str m_sect_name;
    Fmatrix m_attach_offset, m_attach_offset_2;
    Fmatrix m_transform, m_transform_2;
    IKinematicsAnimated *m_model{}, *m_model_2{};
    xr_vector<u16> m_ancors;
    attachable_hud_item* m_attached_items[2]{};
    xr_vector<attachable_hud_item*> m_pool;

public:
	eSecondHandStates second_hand_state = eNoState;

	void SecondHandSwitchState(const eSecondHandStates state);
	void SecondHandOnMotionEnd();

	const u32 SecondHandAnimPlay(const char* section, const char* anm_name, const bool bMixIn = true, const float speed = 1.f);
	bool second_hand_is_pending{};
	u32 curr_second_hand_motion_length{};

	const bool can_play_second_hand_animation() const;
};

extern player_hud* g_player_hud;
