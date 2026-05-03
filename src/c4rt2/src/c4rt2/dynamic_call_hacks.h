/* blAST project
 *
 * Copyright (c) 2025 András Bodor <bodand@pm.me>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the copyright holder nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Originally created: 2025-08-08.
 *
 * src/c4rt2/src/c4rt2/dynamic_call_hacks --
 *   Horrid macro hacks to provide dynamic argument count calling from a
 *   function pointer.
 */
#ifndef BLAST_DYNAMIC_CALL_HACKS_H
#define BLAST_DYNAMIC_CALL_HACKS_H

#include "package.1.h"

static void*
unpad_pointer_1(const c4_ptr64_t ptr);

#define c4_underscores_0(n)
#define c4_underscores_1(n) n, c4_underscores_0(n)
#define c4_underscores_2(n) n, c4_underscores_1(n)
#define c4_underscores_3(n) n, c4_underscores_2(n)
#define c4_underscores_4(n) n, c4_underscores_3(n)
#define c4_underscores_5(n) n, c4_underscores_4(n)
#define c4_underscores_6(n) n, c4_underscores_5(n)
#define c4_underscores_7(n) n, c4_underscores_6(n)
#define c4_underscores_8(n) n, c4_underscores_7(n)
#define c4_underscores_9(n) n, c4_underscores_8(n)
#define c4_underscores_10(n) n, c4_underscores_9(n)
#define c4_underscores_11(n) n, c4_underscores_10(n)
#define c4_underscores_12(n) n, c4_underscores_11(n)
#define c4_underscores_13(n) n, c4_underscores_12(n)
#define c4_underscores_14(n) n, c4_underscores_13(n)
#define c4_underscores_15(n) n, c4_underscores_14(n)
#define c4_underscores_16(n) n, c4_underscores_15(n)
#define c4_underscores_17(n) n, c4_underscores_16(n)
#define c4_underscores_18(n) n, c4_underscores_17(n)
#define c4_underscores_19(n) n, c4_underscores_18(n)
#define c4_underscores_20(n) n, c4_underscores_19(n)
#define c4_underscores_21(n) n, c4_underscores_20(n)
#define c4_underscores_22(n) n, c4_underscores_21(n)
#define c4_underscores_23(n) n, c4_underscores_22(n)
#define c4_underscores_24(n) n, c4_underscores_23(n)
#define c4_underscores_25(n) n, c4_underscores_24(n)
#define c4_underscores_26(n) n, c4_underscores_25(n)
#define c4_underscores_27(n) n, c4_underscores_26(n)
#define c4_underscores_28(n) n, c4_underscores_27(n)
#define c4_underscores_29(n) n, c4_underscores_28(n)
#define c4_underscores_30(n) n, c4_underscores_29(n)
#define c4_underscores_31(n) n, c4_underscores_30(n)
#define c4_underscores_32(n) n, c4_underscores_31(n)
#define c4_underscores_33(n) n, c4_underscores_32(n)
#define c4_underscores_34(n) n, c4_underscores_33(n)
#define c4_underscores_35(n) n, c4_underscores_34(n)
#define c4_underscores_36(n) n, c4_underscores_35(n)
#define c4_underscores_37(n) n, c4_underscores_36(n)
#define c4_underscores_38(n) n, c4_underscores_37(n)
#define c4_underscores_39(n) n, c4_underscores_38(n)
#define c4_underscores_40(n) n, c4_underscores_39(n)
#define c4_underscores_41(n) n, c4_underscores_40(n)
#define c4_underscores_42(n) n, c4_underscores_41(n)
#define c4_underscores_43(n) n, c4_underscores_42(n)
#define c4_underscores_44(n) n, c4_underscores_43(n)
#define c4_underscores_45(n) n, c4_underscores_44(n)
#define c4_underscores_46(n) n, c4_underscores_45(n)
#define c4_underscores_47(n) n, c4_underscores_46(n)
#define c4_underscores_48(n) n, c4_underscores_47(n)
#define c4_underscores_49(n) n, c4_underscores_48(n)
#define c4_underscores_50(n) n, c4_underscores_49(n)
#define c4_underscores_51(n) n, c4_underscores_50(n)
#define c4_underscores_52(n) n, c4_underscores_51(n)
#define c4_underscores_53(n) n, c4_underscores_52(n)
#define c4_underscores_54(n) n, c4_underscores_53(n)
#define c4_underscores_55(n) n, c4_underscores_54(n)
#define c4_underscores_56(n) n, c4_underscores_55(n)
#define c4_underscores_57(n) n, c4_underscores_56(n)
#define c4_underscores_58(n) n, c4_underscores_57(n)
#define c4_underscores_59(n) n, c4_underscores_58(n)
#define c4_underscores_60(n) n, c4_underscores_59(n)
#define c4_underscores_61(n) n, c4_underscores_60(n)
#define c4_underscores_62(n) n, c4_underscores_61(n)
#define c4_underscores_63(n) n, c4_underscores_62(n)
#define c4_underscores_64(n) n, c4_underscores_63(n)
#define c4_underscores_65(n) n, c4_underscores_64(n)
#define c4_underscores_66(n) n, c4_underscores_65(n)
#define c4_underscores_67(n) n, c4_underscores_66(n)
#define c4_underscores_68(n) n, c4_underscores_67(n)
#define c4_underscores_69(n) n, c4_underscores_68(n)
#define c4_underscores_70(n) n, c4_underscores_69(n)
#define c4_underscores_71(n) n, c4_underscores_70(n)
#define c4_underscores_72(n) n, c4_underscores_71(n)
#define c4_underscores_73(n) n, c4_underscores_72(n)
#define c4_underscores_74(n) n, c4_underscores_73(n)
#define c4_underscores_75(n) n, c4_underscores_74(n)
#define c4_underscores_76(n) n, c4_underscores_75(n)
#define c4_underscores_77(n) n, c4_underscores_76(n)
#define c4_underscores_78(n) n, c4_underscores_77(n)
#define c4_underscores_79(n) n, c4_underscores_78(n)
#define c4_underscores_80(n) n, c4_underscores_79(n)
#define c4_underscores_81(n) n, c4_underscores_80(n)
#define c4_underscores_82(n) n, c4_underscores_81(n)
#define c4_underscores_83(n) n, c4_underscores_82(n)
#define c4_underscores_84(n) n, c4_underscores_83(n)
#define c4_underscores_85(n) n, c4_underscores_84(n)
#define c4_underscores_86(n) n, c4_underscores_85(n)
#define c4_underscores_87(n) n, c4_underscores_86(n)
#define c4_underscores_88(n) n, c4_underscores_87(n)
#define c4_underscores_89(n) n, c4_underscores_88(n)
#define c4_underscores_90(n) n, c4_underscores_89(n)
#define c4_underscores_91(n) n, c4_underscores_90(n)
#define c4_underscores_92(n) n, c4_underscores_91(n)
#define c4_underscores_93(n) n, c4_underscores_92(n)
#define c4_underscores_94(n) n, c4_underscores_93(n)
#define c4_underscores_95(n) n, c4_underscores_94(n)
#define c4_underscores_96(n) n, c4_underscores_95(n)
#define c4_underscores_97(n) n, c4_underscores_96(n)
#define c4_underscores_98(n) n, c4_underscores_97(n)
#define c4_underscores_99(n) n, c4_underscores_98(n)
#define c4_underscores_100(n) n, c4_underscores_99(n)
#define c4_underscores_101(n) n, c4_underscores_100(n)
#define c4_underscores_102(n) n, c4_underscores_101(n)
#define c4_underscores_103(n) n, c4_underscores_102(n)
#define c4_underscores_104(n) n, c4_underscores_103(n)
#define c4_underscores_105(n) n, c4_underscores_104(n)
#define c4_underscores_106(n) n, c4_underscores_105(n)
#define c4_underscores_107(n) n, c4_underscores_106(n)
#define c4_underscores_108(n) n, c4_underscores_107(n)
#define c4_underscores_109(n) n, c4_underscores_108(n)
#define c4_underscores_110(n) n, c4_underscores_109(n)
#define c4_underscores_111(n) n, c4_underscores_110(n)
#define c4_underscores_112(n) n, c4_underscores_111(n)
#define c4_underscores_113(n) n, c4_underscores_112(n)
#define c4_underscores_114(n) n, c4_underscores_113(n)
#define c4_underscores_115(n) n, c4_underscores_114(n)
#define c4_underscores_116(n) n, c4_underscores_115(n)
#define c4_underscores_117(n) n, c4_underscores_116(n)
#define c4_underscores_118(n) n, c4_underscores_117(n)
#define c4_underscores_119(n) n, c4_underscores_118(n)
#define c4_underscores_120(n) n, c4_underscores_119(n)
#define c4_underscores_121(n) n, c4_underscores_120(n)
#define c4_underscores_122(n) n, c4_underscores_121(n)
#define c4_underscores_123(n) n, c4_underscores_122(n)
#define c4_underscores_124(n) n, c4_underscores_123(n)
#define c4_underscores_125(n) n, c4_underscores_124(n)
#define c4_underscores_126(n) n, c4_underscores_125(n)
#define c4_underscores_127(n) n, c4_underscores_126(n)
#define c4_underscores_128(n) n, c4_underscores_127(n)
#define c4_underscores_129(n) n, c4_underscores_128(n)

#define c4_underscores(n) c4_underscores_##n(n)

#define c4_dynamic_args0(xver, x, ...)
#define c4_dynamic_args1(xver, x, ...)   _X##xver(127, __VA_ARGS__)c4_dynamic_args0(xver, __VA_ARGS__)
#define c4_dynamic_args2(xver, x, ...)   _X##xver(126, __VA_ARGS__)c4_dynamic_args1(xver, __VA_ARGS__)
#define c4_dynamic_args3(xver, x, ...)   _X##xver(125, __VA_ARGS__)c4_dynamic_args2(xver, __VA_ARGS__)
#define c4_dynamic_args4(xver, x, ...)   _X##xver(124, __VA_ARGS__)c4_dynamic_args3(xver, __VA_ARGS__)
#define c4_dynamic_args5(xver, x, ...)   _X##xver(123, __VA_ARGS__)c4_dynamic_args4(xver, __VA_ARGS__)
#define c4_dynamic_args6(xver, x, ...)   _X##xver(122, __VA_ARGS__)c4_dynamic_args5(xver, __VA_ARGS__)
#define c4_dynamic_args7(xver, x, ...)   _X##xver(121, __VA_ARGS__)c4_dynamic_args6(xver, __VA_ARGS__)
#define c4_dynamic_args8(xver, x, ...)   _X##xver(120, __VA_ARGS__)c4_dynamic_args7(xver, __VA_ARGS__)
#define c4_dynamic_args9(xver, x, ...)   _X##xver(119, __VA_ARGS__)c4_dynamic_args8(xver, __VA_ARGS__)
#define c4_dynamic_args10(xver, x, ...)  _X##xver(118, __VA_ARGS__)c4_dynamic_args9(xver, __VA_ARGS__)
#define c4_dynamic_args11(xver, x, ...)  _X##xver(117, __VA_ARGS__)c4_dynamic_args10(xver, __VA_ARGS__)
#define c4_dynamic_args12(xver, x, ...)  _X##xver(116, __VA_ARGS__)c4_dynamic_args11(xver, __VA_ARGS__)
#define c4_dynamic_args13(xver, x, ...)  _X##xver(115, __VA_ARGS__)c4_dynamic_args12(xver, __VA_ARGS__)
#define c4_dynamic_args14(xver, x, ...)  _X##xver(114, __VA_ARGS__)c4_dynamic_args13(xver, __VA_ARGS__)
#define c4_dynamic_args15(xver, x, ...)  _X##xver(113, __VA_ARGS__)c4_dynamic_args14(xver, __VA_ARGS__)
#define c4_dynamic_args16(xver, x, ...)  _X##xver(112, __VA_ARGS__)c4_dynamic_args15(xver, __VA_ARGS__)
#define c4_dynamic_args17(xver, x, ...)  _X##xver(111, __VA_ARGS__)c4_dynamic_args16(xver, __VA_ARGS__)
#define c4_dynamic_args18(xver, x, ...)  _X##xver(110, __VA_ARGS__)c4_dynamic_args17(xver, __VA_ARGS__)
#define c4_dynamic_args19(xver, x, ...)  _X##xver(109, __VA_ARGS__)c4_dynamic_args18(xver, __VA_ARGS__)
#define c4_dynamic_args20(xver, x, ...)  _X##xver(108, __VA_ARGS__)c4_dynamic_args19(xver, __VA_ARGS__)
#define c4_dynamic_args21(xver, x, ...)  _X##xver(107, __VA_ARGS__)c4_dynamic_args20(xver, __VA_ARGS__)
#define c4_dynamic_args22(xver, x, ...)  _X##xver(106, __VA_ARGS__)c4_dynamic_args21(xver, __VA_ARGS__)
#define c4_dynamic_args23(xver, x, ...)  _X##xver(105, __VA_ARGS__)c4_dynamic_args22(xver, __VA_ARGS__)
#define c4_dynamic_args24(xver, x, ...)  _X##xver(104, __VA_ARGS__)c4_dynamic_args23(xver, __VA_ARGS__)
#define c4_dynamic_args25(xver, x, ...)  _X##xver(103, __VA_ARGS__)c4_dynamic_args24(xver, __VA_ARGS__)
#define c4_dynamic_args26(xver, x, ...)  _X##xver(102, __VA_ARGS__)c4_dynamic_args25(xver, __VA_ARGS__)
#define c4_dynamic_args27(xver, x, ...)  _X##xver(101, __VA_ARGS__)c4_dynamic_args26(xver, __VA_ARGS__)
#define c4_dynamic_args28(xver, x, ...)  _X##xver(100, __VA_ARGS__)c4_dynamic_args27(xver, __VA_ARGS__)
#define c4_dynamic_args29(xver, x, ...)  _X##xver(99, __VA_ARGS__) c4_dynamic_args28(xver, __VA_ARGS__)
#define c4_dynamic_args30(xver, x, ...)  _X##xver(98, __VA_ARGS__) c4_dynamic_args29(xver, __VA_ARGS__)
#define c4_dynamic_args31(xver, x, ...)  _X##xver(97, __VA_ARGS__) c4_dynamic_args30(xver, __VA_ARGS__)
#define c4_dynamic_args32(xver, x, ...)  _X##xver(96, __VA_ARGS__) c4_dynamic_args31(xver, __VA_ARGS__)
#define c4_dynamic_args33(xver, x, ...)  _X##xver(95, __VA_ARGS__) c4_dynamic_args32(xver, __VA_ARGS__)
#define c4_dynamic_args34(xver, x, ...)  _X##xver(94, __VA_ARGS__) c4_dynamic_args33(xver, __VA_ARGS__)
#define c4_dynamic_args35(xver, x, ...)  _X##xver(93, __VA_ARGS__) c4_dynamic_args34(xver, __VA_ARGS__)
#define c4_dynamic_args36(xver, x, ...)  _X##xver(92, __VA_ARGS__) c4_dynamic_args35(xver, __VA_ARGS__)
#define c4_dynamic_args37(xver, x, ...)  _X##xver(91, __VA_ARGS__) c4_dynamic_args36(xver, __VA_ARGS__)
#define c4_dynamic_args38(xver, x, ...)  _X##xver(90, __VA_ARGS__) c4_dynamic_args37(xver, __VA_ARGS__)
#define c4_dynamic_args39(xver, x, ...)  _X##xver(89, __VA_ARGS__) c4_dynamic_args38(xver, __VA_ARGS__)
#define c4_dynamic_args40(xver, x, ...)  _X##xver(88, __VA_ARGS__) c4_dynamic_args39(xver, __VA_ARGS__)
#define c4_dynamic_args41(xver, x, ...)  _X##xver(87, __VA_ARGS__) c4_dynamic_args40(xver, __VA_ARGS__)
#define c4_dynamic_args42(xver, x, ...)  _X##xver(86, __VA_ARGS__) c4_dynamic_args41(xver, __VA_ARGS__)
#define c4_dynamic_args43(xver, x, ...)  _X##xver(85, __VA_ARGS__) c4_dynamic_args42(xver, __VA_ARGS__)
#define c4_dynamic_args44(xver, x, ...)  _X##xver(84, __VA_ARGS__) c4_dynamic_args43(xver, __VA_ARGS__)
#define c4_dynamic_args45(xver, x, ...)  _X##xver(83, __VA_ARGS__) c4_dynamic_args44(xver, __VA_ARGS__)
#define c4_dynamic_args46(xver, x, ...)  _X##xver(82, __VA_ARGS__) c4_dynamic_args45(xver, __VA_ARGS__)
#define c4_dynamic_args47(xver, x, ...)  _X##xver(81, __VA_ARGS__) c4_dynamic_args46(xver, __VA_ARGS__)
#define c4_dynamic_args48(xver, x, ...)  _X##xver(80, __VA_ARGS__) c4_dynamic_args47(xver, __VA_ARGS__)
#define c4_dynamic_args49(xver, x, ...)  _X##xver(79, __VA_ARGS__) c4_dynamic_args48(xver, __VA_ARGS__)
#define c4_dynamic_args50(xver, x, ...)  _X##xver(78, __VA_ARGS__) c4_dynamic_args49(xver, __VA_ARGS__)
#define c4_dynamic_args51(xver, x, ...)  _X##xver(77, __VA_ARGS__) c4_dynamic_args50(xver, __VA_ARGS__)
#define c4_dynamic_args52(xver, x, ...)  _X##xver(76, __VA_ARGS__) c4_dynamic_args51(xver, __VA_ARGS__)
#define c4_dynamic_args53(xver, x, ...)  _X##xver(75, __VA_ARGS__) c4_dynamic_args52(xver, __VA_ARGS__)
#define c4_dynamic_args54(xver, x, ...)  _X##xver(74, __VA_ARGS__) c4_dynamic_args53(xver, __VA_ARGS__)
#define c4_dynamic_args55(xver, x, ...)  _X##xver(73, __VA_ARGS__) c4_dynamic_args54(xver, __VA_ARGS__)
#define c4_dynamic_args56(xver, x, ...)  _X##xver(72, __VA_ARGS__) c4_dynamic_args55(xver, __VA_ARGS__)
#define c4_dynamic_args57(xver, x, ...)  _X##xver(71, __VA_ARGS__) c4_dynamic_args56(xver, __VA_ARGS__)
#define c4_dynamic_args58(xver, x, ...)  _X##xver(70, __VA_ARGS__) c4_dynamic_args57(xver, __VA_ARGS__)
#define c4_dynamic_args59(xver, x, ...)  _X##xver(69, __VA_ARGS__) c4_dynamic_args58(xver, __VA_ARGS__)
#define c4_dynamic_args60(xver, x, ...)  _X##xver(68, __VA_ARGS__) c4_dynamic_args59(xver, __VA_ARGS__)
#define c4_dynamic_args61(xver, x, ...)  _X##xver(67, __VA_ARGS__) c4_dynamic_args60(xver, __VA_ARGS__)
#define c4_dynamic_args62(xver, x, ...)  _X##xver(66, __VA_ARGS__) c4_dynamic_args61(xver, __VA_ARGS__)
#define c4_dynamic_args63(xver, x, ...)  _X##xver(65, __VA_ARGS__) c4_dynamic_args62(xver, __VA_ARGS__)
#define c4_dynamic_args64(xver, x, ...)  _X##xver(64, __VA_ARGS__) c4_dynamic_args63(xver, __VA_ARGS__)
#define c4_dynamic_args65(xver, x, ...)  _X##xver(63, __VA_ARGS__) c4_dynamic_args64(xver, __VA_ARGS__)
#define c4_dynamic_args66(xver, x, ...)  _X##xver(62, __VA_ARGS__) c4_dynamic_args65(xver, __VA_ARGS__)
#define c4_dynamic_args67(xver, x, ...)  _X##xver(61, __VA_ARGS__) c4_dynamic_args66(xver, __VA_ARGS__)
#define c4_dynamic_args68(xver, x, ...)  _X##xver(60, __VA_ARGS__) c4_dynamic_args67(xver, __VA_ARGS__)
#define c4_dynamic_args69(xver, x, ...)  _X##xver(59, __VA_ARGS__) c4_dynamic_args68(xver, __VA_ARGS__)
#define c4_dynamic_args70(xver, x, ...)  _X##xver(58, __VA_ARGS__) c4_dynamic_args69(xver, __VA_ARGS__)
#define c4_dynamic_args71(xver, x, ...)  _X##xver(57, __VA_ARGS__) c4_dynamic_args70(xver, __VA_ARGS__)
#define c4_dynamic_args72(xver, x, ...)  _X##xver(56, __VA_ARGS__) c4_dynamic_args71(xver, __VA_ARGS__)
#define c4_dynamic_args73(xver, x, ...)  _X##xver(55, __VA_ARGS__) c4_dynamic_args72(xver, __VA_ARGS__)
#define c4_dynamic_args74(xver, x, ...)  _X##xver(54, __VA_ARGS__) c4_dynamic_args73(xver, __VA_ARGS__)
#define c4_dynamic_args75(xver, x, ...)  _X##xver(53, __VA_ARGS__) c4_dynamic_args74(xver, __VA_ARGS__)
#define c4_dynamic_args76(xver, x, ...)  _X##xver(52, __VA_ARGS__) c4_dynamic_args75(xver, __VA_ARGS__)
#define c4_dynamic_args77(xver, x, ...)  _X##xver(51, __VA_ARGS__) c4_dynamic_args76(xver, __VA_ARGS__)
#define c4_dynamic_args78(xver, x, ...)  _X##xver(50, __VA_ARGS__) c4_dynamic_args77(xver, __VA_ARGS__)
#define c4_dynamic_args79(xver, x, ...)  _X##xver(49, __VA_ARGS__) c4_dynamic_args78(xver, __VA_ARGS__)
#define c4_dynamic_args80(xver, x, ...)  _X##xver(48, __VA_ARGS__) c4_dynamic_args79(xver, __VA_ARGS__)
#define c4_dynamic_args81(xver, x, ...)  _X##xver(47, __VA_ARGS__) c4_dynamic_args80(xver, __VA_ARGS__)
#define c4_dynamic_args82(xver, x, ...)  _X##xver(46, __VA_ARGS__) c4_dynamic_args81(xver, __VA_ARGS__)
#define c4_dynamic_args83(xver, x, ...)  _X##xver(45, __VA_ARGS__) c4_dynamic_args82(xver, __VA_ARGS__)
#define c4_dynamic_args84(xver, x, ...)  _X##xver(44, __VA_ARGS__) c4_dynamic_args83(xver, __VA_ARGS__)
#define c4_dynamic_args85(xver, x, ...)  _X##xver(43, __VA_ARGS__) c4_dynamic_args84(xver, __VA_ARGS__)
#define c4_dynamic_args86(xver, x, ...)  _X##xver(42, __VA_ARGS__) c4_dynamic_args85(xver, __VA_ARGS__)
#define c4_dynamic_args87(xver, x, ...)  _X##xver(41, __VA_ARGS__) c4_dynamic_args86(xver, __VA_ARGS__)
#define c4_dynamic_args88(xver, x, ...)  _X##xver(40, __VA_ARGS__) c4_dynamic_args87(xver, __VA_ARGS__)
#define c4_dynamic_args89(xver, x, ...)  _X##xver(39, __VA_ARGS__) c4_dynamic_args88(xver, __VA_ARGS__)
#define c4_dynamic_args90(xver, x, ...)  _X##xver(38, __VA_ARGS__) c4_dynamic_args89(xver, __VA_ARGS__)
#define c4_dynamic_args91(xver, x, ...)  _X##xver(37, __VA_ARGS__) c4_dynamic_args90(xver, __VA_ARGS__)
#define c4_dynamic_args92(xver, x, ...)  _X##xver(36, __VA_ARGS__) c4_dynamic_args91(xver, __VA_ARGS__)
#define c4_dynamic_args93(xver, x, ...)  _X##xver(35, __VA_ARGS__) c4_dynamic_args92(xver, __VA_ARGS__)
#define c4_dynamic_args94(xver, x, ...)  _X##xver(34, __VA_ARGS__) c4_dynamic_args93(xver, __VA_ARGS__)
#define c4_dynamic_args95(xver, x, ...)  _X##xver(33, __VA_ARGS__) c4_dynamic_args94(xver, __VA_ARGS__)
#define c4_dynamic_args96(xver, x, ...)  _X##xver(32, __VA_ARGS__) c4_dynamic_args95(xver, __VA_ARGS__)
#define c4_dynamic_args97(xver, x, ...)  _X##xver(31, __VA_ARGS__) c4_dynamic_args96(xver, __VA_ARGS__)
#define c4_dynamic_args98(xver, x, ...)  _X##xver(30, __VA_ARGS__) c4_dynamic_args97(xver, __VA_ARGS__)
#define c4_dynamic_args99(xver, x, ...)  _X##xver(29, __VA_ARGS__) c4_dynamic_args98(xver, __VA_ARGS__)
#define c4_dynamic_args100(xver, x, ...) _X##xver(28, __VA_ARGS__) c4_dynamic_args99(xver, __VA_ARGS__)
#define c4_dynamic_args101(xver, x, ...) _X##xver(27, __VA_ARGS__) c4_dynamic_args100(xver, __VA_ARGS__)
#define c4_dynamic_args102(xver, x, ...) _X##xver(26, __VA_ARGS__) c4_dynamic_args101(xver, __VA_ARGS__)
#define c4_dynamic_args103(xver, x, ...) _X##xver(25, __VA_ARGS__) c4_dynamic_args102(xver, __VA_ARGS__)
#define c4_dynamic_args104(xver, x, ...) _X##xver(24, __VA_ARGS__) c4_dynamic_args103(xver, __VA_ARGS__)
#define c4_dynamic_args105(xver, x, ...) _X##xver(23, __VA_ARGS__) c4_dynamic_args104(xver, __VA_ARGS__)
#define c4_dynamic_args106(xver, x, ...) _X##xver(22, __VA_ARGS__) c4_dynamic_args105(xver, __VA_ARGS__)
#define c4_dynamic_args107(xver, x, ...) _X##xver(21, __VA_ARGS__) c4_dynamic_args106(xver, __VA_ARGS__)
#define c4_dynamic_args108(xver, x, ...) _X##xver(20, __VA_ARGS__) c4_dynamic_args107(xver, __VA_ARGS__)
#define c4_dynamic_args109(xver, x, ...) _X##xver(19, __VA_ARGS__) c4_dynamic_args108(xver, __VA_ARGS__)
#define c4_dynamic_args110(xver, x, ...) _X##xver(18, __VA_ARGS__) c4_dynamic_args109(xver, __VA_ARGS__)
#define c4_dynamic_args111(xver, x, ...) _X##xver(17, __VA_ARGS__) c4_dynamic_args110(xver, __VA_ARGS__)
#define c4_dynamic_args112(xver, x, ...) _X##xver(16, __VA_ARGS__) c4_dynamic_args111(xver, __VA_ARGS__)
#define c4_dynamic_args113(xver, x, ...) _X##xver(15, __VA_ARGS__) c4_dynamic_args112(xver, __VA_ARGS__)
#define c4_dynamic_args114(xver, x, ...) _X##xver(14, __VA_ARGS__) c4_dynamic_args113(xver, __VA_ARGS__)
#define c4_dynamic_args115(xver, x, ...) _X##xver(13, __VA_ARGS__) c4_dynamic_args114(xver, __VA_ARGS__)
#define c4_dynamic_args116(xver, x, ...) _X##xver(12, __VA_ARGS__) c4_dynamic_args115(xver, __VA_ARGS__)
#define c4_dynamic_args117(xver, x, ...) _X##xver(11, __VA_ARGS__) c4_dynamic_args116(xver, __VA_ARGS__)
#define c4_dynamic_args118(xver, x, ...) _X##xver(10, __VA_ARGS__) c4_dynamic_args117(xver, __VA_ARGS__)
#define c4_dynamic_args119(xver, x, ...) _X##xver(9, __VA_ARGS__)  c4_dynamic_args118(xver, __VA_ARGS__)
#define c4_dynamic_args120(xver, x, ...) _X##xver(8, __VA_ARGS__)  c4_dynamic_args119(xver, __VA_ARGS__)
#define c4_dynamic_args121(xver, x, ...) _X##xver(7, __VA_ARGS__)  c4_dynamic_args120(xver, __VA_ARGS__)
#define c4_dynamic_args122(xver, x, ...) _X##xver(6, __VA_ARGS__)  c4_dynamic_args121(xver, __VA_ARGS__)
#define c4_dynamic_args123(xver, x, ...) _X##xver(5, __VA_ARGS__)  c4_dynamic_args122(xver, __VA_ARGS__)
#define c4_dynamic_args124(xver, x, ...) _X##xver(4, __VA_ARGS__)  c4_dynamic_args123(xver, __VA_ARGS__)
#define c4_dynamic_args125(xver, x, ...) _X##xver(3, __VA_ARGS__)  c4_dynamic_args124(xver, __VA_ARGS__)
#define c4_dynamic_args126(xver, x, ...) _X##xver(2, __VA_ARGS__)  c4_dynamic_args125(xver, __VA_ARGS__)
#define c4_dynamic_args127(xver, x, ...) _X##xver(1, __VA_ARGS__)  c4_dynamic_args126(xver, __VA_ARGS__)
#define c4_dynamic_args128(xver, x, ...) _X##xver(0, __VA_ARGS__)   c4_dynamic_args127(xver, __VA_ARGS__)

#define c4_dynamic_args(v, cnt) \
    c4_dynamic_args##cnt(arg, cnt, c4_underscores(cnt))

#define c4_dynamic_call_var(v, cnt) \
    c4_datum_t(*callee)(c4_dynamic_args##cnt(type_ ## v, cnt, c4_underscores(cnt)))

#define c4_dynamic_call_var_ctx(v, cnt) \
    c4_datum_t(*callee)(void* WHEN(cnt)(COMMA) c4_dynamic_args##cnt(type_ ## v, cnt, c4_underscores(cnt)))

#define c4_dynamic_call_fn(v, cnt) \
    [[maybe_unused]] static C4RT_IMPL c4_datum_t \
    c4_dynamic_call_##v##_##cnt(void* pkg_raw) { \
        struct c4_package_v1_t* pkg = pkg_raw; \
        struct c4_package_v1_function_payload* const payload = unpad_pointer_1(pkg->data); \
        c4rt_package_function_t* const calc_fun = payload->calc_fun; \
        struct c4_package_v1_t** args = (void*)payload->args_untyped; \
        c4_dynamic_call_var(v, cnt) = calc_fun; \
        return callee(c4_dynamic_args(v, cnt)); \
    }

#define c4_dynamic_call_fn_ctx(v, cnt) \
    [[maybe_unused]] static C4RT_IMPL c4_datum_t \
    c4_dynamic_call_##v##_ctx_##cnt(void* pkg_raw) { \
        struct c4_package_v1_t* pkg = pkg_raw; \
        struct c4_package_v1_function_payload* const payload = unpad_pointer_1(pkg->data); \
        c4rt_package_function_t* const calc_fun = payload->calc_fun; \
        const size_t pkg_sz = sizeof(struct c4_package_v1_t); \
        void* ctx = payload->args_untyped; \
        struct c4_package_v1_t** args = (void*)(payload->args_untyped \
                                                + (ptrdiff_t)pkg->context_sz_divided_bytes \
                                                /*        */ * pkg_sz); \
        (void)args; /* w/o this may trigger warnings if v=0 */ \
        c4_dynamic_call_var_ctx(v, cnt) = calc_fun; \
        return callee(ctx WHEN(cnt)(COMMA) c4_dynamic_args(v, cnt)); \
    }

#define c4_dynamic_call_fn_all(v, cnt) \
    c4_dynamic_call_fn(v, cnt) \
    c4_dynamic_call_fn_ctx(v, cnt)

#define HEAD(...) HEAD_I(__VA_ARGS__)
#define HEAD_I(x, ...) x
#define IS_EMPTY(...) HEAD(IS_EMPTY_I(__VA_ARGS__))
#define IS_EMPTY_I(...) __VA_OPT__(0,) 1

#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,)
#define PROBE(x) x, 1,

#define COMMA ,
#define NOTHING

#define CAT_I(x, ...) x ## __VA_ARGS__

#define _Xtype_v1(x,y,...) struct c4_package_v1_t* IIF(IS_EMPTY(__VA_ARGS__))(NOTHING, COMMA)
#define _Xarg(x,y,...) args[y-128+x] IIF(IS_EMPTY(__VA_ARGS__))(NOTHING, COMMA)

#define _Xcase_v1(x,y,...) case (y-128+x): [[clang::musttail]] return c4_dynamic_call_v1_##x(pkg_raw);
#define _Xcase_v1_ctx(x,y,...) case (y-128+x): [[clang::musttail]] return c4_dynamic_call_v1_ctx_##x(pkg_raw);

#define IIF(c) IIF_I(c)
#define IIF_I(c) IIF_##c
#define IIF_0(t,f) f
#define IIF_1(t,f) t

#define COMPL(b) CAT_I(COMPL_, b)
#define COMPL_0 1
#define COMPL_1 0

#define NOT(x) CHECK(CAT_I(NOT_, x))
#define NOT_0 PROBE(~)

#define BOOL(x) COMPL(NOT(x))
#define IF(c) IIF(BOOL(c))

#define EAT(...)
#define EXPAND(...) __VA_ARGS__
#define WHEN(c) IF(c)(EXPAND, EAT)

c4_dynamic_call_fn_all(v1, 0)
c4_dynamic_call_fn_all(v1, 1)
c4_dynamic_call_fn_all(v1, 2)
c4_dynamic_call_fn_all(v1, 3)
c4_dynamic_call_fn_all(v1, 4)
c4_dynamic_call_fn_all(v1, 5)
c4_dynamic_call_fn_all(v1, 6)
c4_dynamic_call_fn_all(v1, 7)
c4_dynamic_call_fn_all(v1, 8)
c4_dynamic_call_fn_all(v1, 9)
c4_dynamic_call_fn_all(v1, 10)
c4_dynamic_call_fn_all(v1, 11)
c4_dynamic_call_fn_all(v1, 12)
c4_dynamic_call_fn_all(v1, 13)
c4_dynamic_call_fn_all(v1, 14)
c4_dynamic_call_fn_all(v1, 15)
c4_dynamic_call_fn_all(v1, 16)
c4_dynamic_call_fn_all(v1, 17)
c4_dynamic_call_fn_all(v1, 18)
c4_dynamic_call_fn_all(v1, 19)
c4_dynamic_call_fn_all(v1, 20)
c4_dynamic_call_fn_all(v1, 21)
c4_dynamic_call_fn_all(v1, 22)
c4_dynamic_call_fn_all(v1, 23)
c4_dynamic_call_fn_all(v1, 24)
c4_dynamic_call_fn_all(v1, 25)
c4_dynamic_call_fn_all(v1, 26)
c4_dynamic_call_fn_all(v1, 27)
c4_dynamic_call_fn_all(v1, 28)
c4_dynamic_call_fn_all(v1, 29)
c4_dynamic_call_fn_all(v1, 30)
c4_dynamic_call_fn_all(v1, 31)
c4_dynamic_call_fn_all(v1, 32)
c4_dynamic_call_fn_all(v1, 33)
c4_dynamic_call_fn_all(v1, 34)
c4_dynamic_call_fn_all(v1, 35)
c4_dynamic_call_fn_all(v1, 36)
c4_dynamic_call_fn_all(v1, 37)
c4_dynamic_call_fn_all(v1, 38)
c4_dynamic_call_fn_all(v1, 39)
c4_dynamic_call_fn_all(v1, 40)
c4_dynamic_call_fn_all(v1, 41)
c4_dynamic_call_fn_all(v1, 42)
c4_dynamic_call_fn_all(v1, 43)
c4_dynamic_call_fn_all(v1, 44)
c4_dynamic_call_fn_all(v1, 45)
c4_dynamic_call_fn_all(v1, 46)
c4_dynamic_call_fn_all(v1, 47)
c4_dynamic_call_fn_all(v1, 48)
c4_dynamic_call_fn_all(v1, 49)
c4_dynamic_call_fn_all(v1, 50)
c4_dynamic_call_fn_all(v1, 51)
c4_dynamic_call_fn_all(v1, 52)
c4_dynamic_call_fn_all(v1, 53)
c4_dynamic_call_fn_all(v1, 54)
c4_dynamic_call_fn_all(v1, 55)
c4_dynamic_call_fn_all(v1, 56)
c4_dynamic_call_fn_all(v1, 57)
c4_dynamic_call_fn_all(v1, 58)
c4_dynamic_call_fn_all(v1, 59)
c4_dynamic_call_fn_all(v1, 60)
c4_dynamic_call_fn_all(v1, 61)
c4_dynamic_call_fn_all(v1, 62)
c4_dynamic_call_fn_all(v1, 63)
c4_dynamic_call_fn_all(v1, 64)
c4_dynamic_call_fn_all(v1, 65)
c4_dynamic_call_fn_all(v1, 66)
c4_dynamic_call_fn_all(v1, 67)
c4_dynamic_call_fn_all(v1, 68)
c4_dynamic_call_fn_all(v1, 69)
c4_dynamic_call_fn_all(v1, 70)
c4_dynamic_call_fn_all(v1, 71)
c4_dynamic_call_fn_all(v1, 72)
c4_dynamic_call_fn_all(v1, 73)
c4_dynamic_call_fn_all(v1, 74)
c4_dynamic_call_fn_all(v1, 75)
c4_dynamic_call_fn_all(v1, 76)
c4_dynamic_call_fn_all(v1, 77)
c4_dynamic_call_fn_all(v1, 78)
c4_dynamic_call_fn_all(v1, 79)
c4_dynamic_call_fn_all(v1, 80)
c4_dynamic_call_fn_all(v1, 81)
c4_dynamic_call_fn_all(v1, 82)
c4_dynamic_call_fn_all(v1, 83)
c4_dynamic_call_fn_all(v1, 84)
c4_dynamic_call_fn_all(v1, 85)
c4_dynamic_call_fn_all(v1, 86)
c4_dynamic_call_fn_all(v1, 87)
c4_dynamic_call_fn_all(v1, 88)
c4_dynamic_call_fn_all(v1, 89)
c4_dynamic_call_fn_all(v1, 90)
c4_dynamic_call_fn_all(v1, 91)
c4_dynamic_call_fn_all(v1, 92)
c4_dynamic_call_fn_all(v1, 93)
c4_dynamic_call_fn_all(v1, 94)
c4_dynamic_call_fn_all(v1, 95)
c4_dynamic_call_fn_all(v1, 96)
c4_dynamic_call_fn_all(v1, 97)
c4_dynamic_call_fn_all(v1, 98)
c4_dynamic_call_fn_all(v1, 99)
c4_dynamic_call_fn_all(v1, 100)
c4_dynamic_call_fn_all(v1, 101)
c4_dynamic_call_fn_all(v1, 102)
c4_dynamic_call_fn_all(v1, 103)
c4_dynamic_call_fn_all(v1, 104)
c4_dynamic_call_fn_all(v1, 105)
c4_dynamic_call_fn_all(v1, 106)
c4_dynamic_call_fn_all(v1, 107)
c4_dynamic_call_fn_all(v1, 108)
c4_dynamic_call_fn_all(v1, 109)
c4_dynamic_call_fn_all(v1, 110)
c4_dynamic_call_fn_all(v1, 111)
c4_dynamic_call_fn_all(v1, 112)
c4_dynamic_call_fn_all(v1, 113)
c4_dynamic_call_fn_all(v1, 114)
c4_dynamic_call_fn_all(v1, 115)
c4_dynamic_call_fn_all(v1, 116)
c4_dynamic_call_fn_all(v1, 117)
c4_dynamic_call_fn_all(v1, 118)
c4_dynamic_call_fn_all(v1, 119)
c4_dynamic_call_fn_all(v1, 120)
c4_dynamic_call_fn_all(v1, 121)
c4_dynamic_call_fn_all(v1, 122)
c4_dynamic_call_fn_all(v1, 123)
c4_dynamic_call_fn_all(v1, 124)
c4_dynamic_call_fn_all(v1, 125)
c4_dynamic_call_fn_all(v1, 126)
c4_dynamic_call_fn_all(v1, 127)
c4_dynamic_call_fn_all(v1, 128)

#define c4_dynamic_cases(max) c4_dynamic_args##max(case_v1, max, c4_underscores(max))
#define c4_dynamic_cases_ctx(max) c4_dynamic_args##max(case_v1_ctx, max, c4_underscores(max))

static C4RT_IMPL c4_datum_t

c4_dynamic_call_v1(void* pkg_raw) {
	const struct c4_package_v1_t* const pkg = pkg_raw;
	const uint16_t arity = pkg->function_arity;
	assert(arity <= 128u && "not implemented: c4rt2 cannot call functions "
	       "with more than 128 arguments");

	switch (arity) {
	c4_dynamic_cases(128)
	default: ;
	}
	C4_UNREACHABLE;
}

static C4RT_IMPL c4_datum_t

c4_dynamic_call_v1_ctx(void* pkg_raw) {
	const struct c4_package_v1_t* const pkg = pkg_raw;
	const uint16_t arity = pkg->function_arity;
	assert(arity <= 128u && "not implemented: c4rt2 cannot call functions "
	       "with more than 128 arguments");

	switch (arity) {
	c4_dynamic_cases_ctx(128)
	default: ;
	}
	C4_UNREACHABLE;
}

#undef _Xcase_v1
#undef _Xcase_v1_ctx
#undef _Xtype_v1
#undef _Xarg

#endif
