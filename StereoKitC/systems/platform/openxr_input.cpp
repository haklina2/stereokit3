#include "openxr.h"
#include "openxr_input.h"

#include "../../stereokit.h"
#include "../../_stereokit.h"

#include <openxr/openxr.h>

namespace sk {

///////////////////////////////////////////

XrActionSet xrc_action_set;
XrAction    xrc_pose_action;
XrAction    xrc_point_action;
XrAction    xrc_select_action;
XrAction    xrc_grip_action;
XrAction    xrc_gaze_action;
XrSpace     xrc_gaze_space;
XrPath      xrc_hand_subaction_path[2];
XrSpace     xrc_point_space[2];
XrSpace     xr_hand_space[2] = {};

///////////////////////////////////////////

bool oxri_init() {
	XrActionSetCreateInfo actionset_info = { XR_TYPE_ACTION_SET_CREATE_INFO };
	strcpy_s(actionset_info.actionSetName,          "input");
	strcpy_s(actionset_info.localizedActionSetName, "Input");
	xr_check(xrCreateActionSet(xr_instance, &actionset_info, &xrc_action_set),
		"xrCreateActionSet failed: [%s]");
	xrStringToPath(xr_instance, "/user/hand/left",  &xrc_hand_subaction_path[0]);
	xrStringToPath(xr_instance, "/user/hand/right", &xrc_hand_subaction_path[1]);

	// Create an action to track the position and orientation of the hands! This is
	// the controller location, or the center of the palms for actual hands.
	XrActionCreateInfo action_info = { XR_TYPE_ACTION_CREATE_INFO };
	action_info.countSubactionPaths = _countof(xrc_hand_subaction_path);
	action_info.subactionPaths      = xrc_hand_subaction_path;
	action_info.actionType          = XR_ACTION_TYPE_POSE_INPUT;
	strcpy_s(action_info.actionName,          "hand_pose");
	strcpy_s(action_info.localizedActionName, "Hand Pose");
	xr_check(xrCreateAction(xrc_action_set, &action_info, &xrc_pose_action),
		"xrCreateAction failed: [%s]");

	// Create an action to track the pointing position and orientation!
	action_info.actionType = XR_ACTION_TYPE_POSE_INPUT;
	strcpy_s(action_info.actionName,          "hand_point");
	strcpy_s(action_info.localizedActionName, "Hand Point");
	xr_check(xrCreateAction(xrc_action_set, &action_info, &xrc_point_action),
		"xrCreateAction failed: [%s]");

	// Create an action for listening to the select action! This is primary trigger
	// on controllers, and an airtap on HoloLens
	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	strcpy_s(action_info.actionName,          "select");
	strcpy_s(action_info.localizedActionName, "Select");
	xr_check(xrCreateAction(xrc_action_set, &action_info, &xrc_select_action),
		"xrCreateAction failed: [%s]");

	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	strcpy_s(action_info.actionName,          "grip");
	strcpy_s(action_info.localizedActionName, "Grip");
	xr_check(xrCreateAction(xrc_action_set, &action_info, &xrc_grip_action),
		"xrCreateAction failed: [%s]");

	// eye gaze
	action_info.actionType = XR_ACTION_TYPE_POSE_INPUT;
	strcpy_s(action_info.actionName,          "gaze");
	strcpy_s(action_info.localizedActionName, "Gaze");
	xr_check(xrCreateAction(xrc_action_set, &action_info, &xrc_gaze_action),
		"xrCreateAction failed: [%s]");

	// Bind the actions we just created to specific locations on the Khronos simple_controller
	// definition! These are labeled as 'suggested' because they may be overridden by the runtime
	// preferences. For example, if the runtime allows you to remap buttons, or provides input
	// accessibility settings.
	XrPath profile_path;
	XrPath pose_path  [2];
	XrPath point_path [2];
	XrPath select_path[2];
	XrPath grip_path  [2];
	xrStringToPath(xr_instance, "/user/hand/left/input/grip/pose",  &pose_path[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/grip/pose", &pose_path[1]);
	xrStringToPath(xr_instance, "/user/hand/left/input/aim/pose",   &point_path[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/aim/pose",  &point_path[1]);
	XrInteractionProfileSuggestedBinding suggested_binds = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };

	// microsoft / motion_controller
	{
		xrStringToPath(xr_instance, "/user/hand/left/input/trigger/value",  &select_path[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/trigger/value", &select_path[1]);
		xrStringToPath(xr_instance, "/user/hand/left/input/squeeze/click",  &grip_path[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/squeeze/click", &grip_path[1]);
		XrActionSuggestedBinding bindings[] = {
			{ xrc_pose_action,   pose_path  [0] }, { xrc_pose_action,   pose_path  [1] },
			{ xrc_point_action,  point_path [0] }, { xrc_point_action,  point_path [1] },
			{ xrc_select_action, select_path[0] }, { xrc_select_action, select_path[1] },
			{ xrc_grip_action,   grip_path  [0] }, { xrc_grip_action,   grip_path  [1] },
		};

		xrStringToPath(xr_instance, "/interaction_profiles/microsoft/motion_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = &bindings[0];
		suggested_binds.countSuggestedBindings = _countof(bindings);
		xr_check(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds),
			"xrSuggestInteractionProfileBindings failed: [%s]");
	}

	// khr / simple_controller
	{
		xrStringToPath(xr_instance, "/user/hand/left/input/select/click",  &select_path[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/select/click", &select_path[1]);
		XrActionSuggestedBinding bindings[] = {
			{ xrc_pose_action,   pose_path  [0] }, { xrc_pose_action,   pose_path  [1] },
			{ xrc_point_action,  point_path [0] }, { xrc_point_action,  point_path [1] },
			{ xrc_select_action, select_path[0] }, { xrc_select_action, select_path[1] },
		};

		xrStringToPath(xr_instance, "/interaction_profiles/khr/simple_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = &bindings[0];
		suggested_binds.countSuggestedBindings = _countof(bindings);
		xr_check(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds),
			"xrSuggestInteractionProfileBindings failed: [%s]");
	}

	// eye gaze extension
	if (xr_eye_gaze)
	{
		XrPath gaze_path;
		xrStringToPath(xr_instance, "/user/eyes_ext/input/gaze_ext/pose", &gaze_path);
		XrActionSuggestedBinding bindings[] = {
			{ xrc_gaze_action, gaze_path },
		};

		xrStringToPath(xr_instance, "/interaction_profiles/ext/eye_gaze_interaction", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = &bindings[0];
		suggested_binds.countSuggestedBindings = _countof(bindings);
		xr_check(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds),
			"xrSuggestInteractionProfileBindings failed: [%s]");

		XrActionSpaceCreateInfo action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.action            = xrc_gaze_action;
		xr_check(xrCreateActionSpace(xr_session, &action_space_info, &xrc_gaze_space),
			"xrCreateActionSpace failed: [%s]");
	}

	// Create frames of reference for the pose actions
	XrActionSpaceCreateInfo action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
	for (int32_t i = 0; i < 2; i++) {
		action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = xrc_hand_subaction_path[i];
		action_space_info.action            = xrc_pose_action;
		xr_check(xrCreateActionSpace(xr_session, &action_space_info, &xr_hand_space[i]),
			"xrCreateActionSpace failed: [%s]");

		action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = xrc_hand_subaction_path[i];
		action_space_info.action            = xrc_point_action;
		xr_check(xrCreateActionSpace(xr_session, &action_space_info, &xrc_point_space[i]),
			"xrCreateActionSpace failed: [%s]");
	}
	
	// Attach the action set we just made to the session
	XrSessionActionSetsAttachInfo attach_info = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attach_info.countActionSets = 1;
	attach_info.actionSets      = &xrc_action_set;
	xr_check(xrAttachSessionActionSets(xr_session, &attach_info),
		"xrAttachSessionActionSets failed: [%s]");

	return true;
}

///////////////////////////////////////////

void oxri_shutdown() {
	xrDestroySpace(xrc_gaze_space);
	xrDestroySpace(xr_hand_space[0]);
	xrDestroySpace(xr_hand_space[1]);
	xrDestroyActionSet(xrc_action_set);
}

///////////////////////////////////////////

void oxri_update_frame() {
	// Update our action set with up-to-date input data!
	XrActiveActionSet action_set = { };
	action_set.actionSet     = xrc_action_set;
	action_set.subactionPath = XR_NULL_PATH;
	XrActionsSyncInfo sync_info = { XR_TYPE_ACTIONS_SYNC_INFO };
	sync_info.countActiveActionSets = 1;
	sync_info.activeActionSets      = &action_set;
	xrSyncActions(xr_session, &sync_info);

	if (xr_eye_gaze) {
		XrActionStatePose    actionStatePose = {XR_TYPE_ACTION_STATE_POSE};
		XrActionStateGetInfo getActionStateInfo = {XR_TYPE_ACTION_STATE_GET_INFO};
		getActionStateInfo.action = xrc_gaze_action;
		xrGetActionStatePose(xr_session, &getActionStateInfo, &actionStatePose);

		if(actionStatePose.isActive){
			pose_t gaze_pose;
			openxr_get_space(xrc_gaze_space, gaze_pose);

			vec3 dir = gaze_pose.orientation * vec3_forward;
			log_infof("%.2f, %.2f, %.2f", dir.x, dir.y, dir.z);
			line_add(gaze_pose.position + gaze_pose.orientation * vec3_forward, vec3_zero, { 255,255,255,255 }, { 255, 255, 255, 255 }, 0.01f);
		} else {
			log_infof("Gaze not active");
		}
	}
}

} // namespace sk
