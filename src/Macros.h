#pragma once

#define EXPAND(x) x

#define COUNT_ARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
	_17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31,                 \
	_32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46,                 \
	_47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61,                 \
	_62, _63, _64, N, ...) N
#define COUNT_ARGS(...) COUNT_ARGS_IMPL(__VA_ARGS__, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
	49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35,                                                  \
	34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20,                                                  \
	19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define APPLY_TO_STRUCT_MEMBERS(Macro, ...) \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL(Macro, COUNT_ARGS(__VA_ARGS__), __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL(Macro, N, ...) \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_N(Macro, N, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_N(Macro, N, ...) \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_##N(Macro, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_1(Macro, Member) \
	EXPAND(Macro(Member))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_2(Macro, Member, ...) \
	EXPAND(Macro(Member))                                  \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_1(Macro, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_3(Macro, Member, ...) \
	EXPAND(Macro(Member))                                  \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_2(Macro, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_4(Macro, Member, ...) \
	EXPAND(Macro(Member))                                  \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_3(Macro, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_5(Macro, Member, ...) \
	EXPAND(Macro(Member))                                  \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_4(Macro, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_6(Macro, Member, ...) \
	EXPAND(Macro(Member))                                  \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_5(Macro, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_7(Macro, Member, ...) \
	EXPAND(Macro(Member))                                  \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_6(Macro, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_8(Macro, Member, ...) \
	EXPAND(Macro(Member))                                  \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_7(Macro, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_9(Macro, Member, ...) \
	EXPAND(Macro(Member))                                  \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_8(Macro, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_10(Macro, Member, ...) \
	EXPAND(Macro(Member))                                  \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_9(Macro, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_11(Macro, Member, ...) \
	EXPAND(Macro(Member))                                  \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_10(Macro, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_12(Macro, Member, ...) \
	EXPAND(Macro(Member))                                  \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_11(Macro, __VA_ARGS__))

#define APPLY_TO_STRUCT_MEMBERS_IMPL_13(Macro, Member, ...) \
	EXPAND(Macro(Member))                                   \
	EXPAND(APPLY_TO_STRUCT_MEMBERS_IMPL_12(Macro, __VA_ARGS__))

// Add more APPLY_TO_STRUCT_MEMBERS_IMPL_N macros as needed for larger argument counts

#define GENERATE_LERP_MEMBER(Member) \
	Member = Interpolate(startSettings->Member.value(), endSettings->Member.value(), t);

#define GENERATE_OVERRIDE_MEMBER(Member)  \
	if (newSettings->Member.has_value()) { \
		Member = newSettings->Member;      \
	} \

#define GENERATE_RESET_MEMBER(Member)   \
		Member = std::nullopt;			   \


template <typename T>
T Interpolate(const T& start, const T& end, double progress)
{
	if (start == end)
		return end;
	else {
		double ret =  start + (end - start) * progress;
		return T(ret);
	}
}