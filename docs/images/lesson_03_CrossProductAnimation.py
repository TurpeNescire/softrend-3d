from manim import *
import numpy as np

# ── Palette ───────────────────────────────────────────────────────────────────
C_A        = YELLOW
C_B        = BLUE
C_C        = GREEN
C_U        = BLUE_B
C_V        = GREEN_B
C_PARA     = "#2a9d8f"
C_PARA_NEG = "#e76f51"
C_TRI      = "#e9c46a"
C_NARR     = WHITE
C_FORM     = LIGHT_GREY
C_WIND     = BLACK
C_PARA_LBL = "#4fc3f7"
C_TRI_LBL  = YELLOW
C_ZVEC_POS = "#f4a261"
C_ZVEC_NEG = "#e76f51"
MONO       = "Courier New"

NARR_Y     = DOWN * 3.2
FORM_LEFT  = -6.9
FORM_TOP_Y =  3.6

GRID_SCALE = 1.18
GRID_X0    = -0.8
GRID_Y0    = -2.2

def pt(gx, gy):
    return np.array([GRID_X0 + gx * GRID_SCALE,
                     GRID_Y0 + gy * GRID_SCALE, 0])

PA = pt(1, 1);  PB = pt(4, 1);  PC = pt(2, 4)
PO = pt(0, 0);  PD = PB + PC - PA
PARA_CENTROID = (PA + PB + PD + PC) / 4

def T(text, size=20, color=C_FORM, bold=False):
    w = BOLD if bold else NORMAL
    return Text(text, font_size=size, color=color, font=MONO, weight=w)

def place_col(items, top_y, left_x=FORM_LEFT, gap=0.26):
    y = top_y
    for m in items:
        m.move_to([left_x, y, 0], aligned_edge=LEFT)
        y -= m.height + gap
    return items


class CrossProductAnimation(Scene):
    def construct(self):

        # narration helper — plain Scene, FadeIn/FadeOut work normally
        def narration(text, prev=None, rt=0.5):
            t = T(text, size=18, color=C_NARR).move_to(NARR_Y)
            anims = [FadeIn(t, run_time=rt)]
            if prev is not None:
                anims.append(FadeOut(prev, run_time=rt))
            self.play(*anims)
            return t

        # ── FRAME 1 ───────────────────────────────────────────────────────────
        title = T("Triangle Area via Cross Product", size=44, color=WHITE)
        self.play(FadeIn(title))
        self.wait(3)
        self.play(FadeOut(title))

        # ── FRAME 2: Grid + labelled points ───────────────────────────────────
        plane = NumberPlane(
            x_range=[-0.3, 5.3, 1], y_range=[-0.3, 5.3, 1],
            x_length=5.6 * GRID_SCALE, y_length=5.6 * GRID_SCALE,
            background_line_style={"stroke_color": GREY_D, "stroke_width": 1},
            axis_config={"stroke_color": GREY_B, "stroke_width": 2},
        ).move_to([GRID_X0 + 2.5 * GRID_SCALE, GRID_Y0 + 2.5 * GRID_SCALE, 0])

        dot_a = Dot(PA, color=C_A, radius=0.09)
        dot_b = Dot(PB, color=C_B, radius=0.09)
        dot_c = Dot(PC, color=C_C, radius=0.09)
        lbl_a = T("a=(1,1)", size=17, color=C_A).next_to(PA, DOWN, buff=0.14).shift(RIGHT * 0.59)
        lbl_b = T("b=(4,1)", size=17, color=C_B).next_to(PB, DR, buff=0.12)
        lbl_c = T("c=(2,4)", size=17, color=C_C).next_to(PC, UR, buff=0.12)

        self.play(Create(plane), run_time=1.2)
        self.play(FadeIn(dot_a), FadeIn(lbl_a),
                  FadeIn(dot_b), FadeIn(lbl_b),
                  FadeIn(dot_c), FadeIn(lbl_c))
        self.wait(1)

        # ── FRAME 3 ───────────────────────────────────────────────────────────
        narr = narration(
            "Three position vectors — each rooted at the screen origin.")

        tri_outline = Polygon(PA, PB, PC, fill_opacity=0,
                              stroke_color=WHITE, stroke_opacity=0.25,
                              stroke_width=1.5)
        arr_a = Arrow(PO, PA, buff=0, color=C_A, stroke_width=3,
                      max_tip_length_to_length_ratio=0.12)
        arr_b = Arrow(PO, PB, buff=0, color=C_B, stroke_width=3,
                      max_tip_length_to_length_ratio=0.12)
        arr_c = Arrow(PO, PC, buff=0, color=C_C, stroke_width=3,
                      max_tip_length_to_length_ratio=0.12)
        dot_o = Dot(PO, color=WHITE, radius=0.08)

        self.play(FadeIn(tri_outline))
        self.play(FadeIn(dot_o), GrowArrow(arr_a))
        self.play(GrowArrow(arr_b))
        self.play(GrowArrow(arr_c))
        self.wait(6.5)

        # ── FRAME 4a ──────────────────────────────────────────────────────────
        narr = narration(
            "The cross product needs two vectors from a shared origin.", narr)
        self.wait(5)

        # ── FRAME 4b ──────────────────────────────────────────────────────────
        narr = narration(
            "We can choose any vector to be the origin — let's pick vector a.", narr)
        ring = Circle(radius=0.16, color=RED, stroke_width=2.5).move_to(PA)
        self.play(Create(ring))
        self.play(arr_b.animate.set_opacity(0.25),
                  arr_c.animate.set_opacity(0.25))
        self.wait(5)
        self.play(FadeOut(ring))

        # ── FRAME 4c ──────────────────────────────────────────────────────────
        narr = narration(
            "Vectors b and c are the new displacement vectors from vector a as their origin.",
            narr)
        self.wait(3)
        self.play(arr_a.animate.set_opacity(0.25))

        # ── FRAME 5: u = b - a ────────────────────────────────────────────────
        narr = narration(
            "The displacement vector u from a to b is found by subtracting a from b.", narr)

        arr_u   = Arrow(PA, PB, buff=0, color=C_U, stroke_width=4,
                        max_tip_length_to_length_ratio=0.12)
        u_coord = T("u=(3,0)", size=17, color=C_U).next_to(PB, DR, buff=0.10)

        f_u0 = T("Finding vector u:", size=20, color=C_FORM)
        f_u1 = T("u = b - a",            size=20, color=C_FORM)
        f_u1b= T("= (b.x-a.x, b.y-a.y)", size=20, color=C_FORM)
        f_u2 = T("= (4-1, 1-1)",          size=20, color=C_FORM)
        f_u3 = T("= (3, 0)",               size=20, color=C_U)
        place_col([f_u0, f_u1, f_u1b, f_u2, f_u3], FORM_TOP_Y)
        # Shift sub-lines so their = aligns with the = in f_u1
        # "u " is 2 chars; at size=20 monospace each char ~0.135 units
        _u_eq_x = f_u1.get_left()[0] + 0.27
        for _m in [f_u1b, f_u2, f_u3]:
            _m.align_to(f_u1, LEFT)
            _m.shift(RIGHT * (_u_eq_x - _m.get_left()[0]))

        self.play(GrowArrow(arr_u), FadeOut(tri_outline))
        # Create lbl_a_new here so it appears with the b→u swap
        lbl_a_new = T("(0,0)", size=17, color=C_A)            .align_to(lbl_a, LEFT).align_to(lbl_a, UP)
        lbl_a_ghost = lbl_a.copy().set_opacity(1)
        self.add(lbl_a_ghost)
        _a_target = lbl_a_new.get_center() + DOWN * (lbl_a_new.height / 2 + lbl_a_ghost.height / 2 + 0.08)
        lbl_b_ghost = lbl_b.copy().set_opacity(1)
        self.add(lbl_b_ghost)
        _b_target = u_coord.get_center() + DOWN * (u_coord.height / 2 + lbl_b_ghost.height / 2 + 0.08)
        self.play(
            FadeOut(lbl_a), FadeIn(lbl_a_new),
            lbl_a_ghost.animate.move_to(_a_target),
            FadeOut(lbl_b), FadeIn(u_coord),
            lbl_b_ghost.animate.move_to(_b_target),
            run_time=0.8,
        )
        self.play(
            lbl_a_ghost.animate.set_opacity(0.3),
            lbl_b_ghost.animate.set_opacity(0.3),
            run_time=0.5,
        )
        self.play(FadeIn(f_u0), FadeIn(f_u1))
        self.play(FadeIn(f_u1b))
        self.play(FadeIn(f_u2), FadeIn(f_u3))
        self.wait(7)

        # ── FRAME 6: v = c - a ────────────────────────────────────────────────
        narr = narration(
            "The displacement vector v from a to c is found by subtracting a from c.", narr)

        arr_v   = Arrow(PA, PC, buff=0, color=C_V, stroke_width=4,
                        max_tip_length_to_length_ratio=0.12)
        v_coord = T("v=(1,3)", size=17, color=C_V).next_to(PC, UR, buff=0.10)

        u_bot = f_u3.get_bottom()[1] - 0.60
        f_v0 = T("Finding vector v:", size=20, color=C_FORM)
        f_v1 = T("v = c - a",            size=20, color=C_FORM)
        f_v1b= T("= (c.x-a.x, c.y-a.y)", size=20, color=C_FORM)
        f_v2 = T("= (2-1, 4-1)",          size=20, color=C_FORM)
        f_v3 = T("= (1, 3)",               size=20, color=C_V)
        place_col([f_v0, f_v1, f_v1b, f_v2, f_v3], u_bot)
        _v_eq_x = f_v1.get_left()[0] + 0.27
        for _m in [f_v1b, f_v2, f_v3]:
            _m.align_to(f_v1, LEFT)
            _m.shift(RIGHT * (_v_eq_x - _m.get_left()[0]))

        self.play(GrowArrow(arr_v))
        lbl_c_ghost = lbl_c.copy().set_opacity(1)
        self.add(lbl_c_ghost)
        _c_target = v_coord.get_center() + UP * (v_coord.height / 2 + lbl_c_ghost.height / 2 + 0.08)
        self.play(
            FadeOut(lbl_c),
            FadeIn(v_coord),
            lbl_c_ghost.animate.move_to(_c_target),
            run_time=0.8,
        )
        self.play(lbl_c_ghost.animate.set_opacity(0.3), run_time=0.5)
        self.play(FadeIn(f_v0), FadeIn(f_v1))
        self.play(FadeIn(f_v1b))
        self.play(FadeIn(f_v2), FadeIn(f_v3))
        self.wait(7)

        # ── FRAME 7: Dashed origin lines ──────────────────────────────────────
        narr = narration(
            "u and v are two new vectors from a common origin at a.", narr)

        dash_a = DashedLine(PO, PA, color=GREY_C, dash_length=0.12, stroke_width=1.5)
        dash_b = DashedLine(PO, PB, color=GREY_C, dash_length=0.12, stroke_width=1.5)
        dash_c = DashedLine(PO, PC, color=GREY_C, dash_length=0.12, stroke_width=1.5)
        self.play(
            FadeOut(arr_a), FadeOut(arr_b), FadeOut(arr_c), FadeOut(dot_o),
            FadeIn(dash_a), FadeIn(dash_b), FadeIn(dash_c))
        self.wait(5)
        self.play(FadeOut(lbl_a_ghost), FadeOut(lbl_b_ghost), FadeOut(lbl_c_ghost))

        # ── FRAME 8: Collapse + 3D notation + parallelogram ───────────────────
        narr = narration(
            "u and v lie flat in the xy plane. We treat them as 3D by setting z=0.", narr)

        f_u1c = T("u = b - a = (3, 0, 0)", size=19, color=C_U)
        f_v1c = T("v = c - a = (1, 3, 0)", size=19, color=C_V)
        place_col([f_u1c, f_v1c], FORM_TOP_Y)

        self.play(
            FadeOut(f_u0), FadeOut(f_u1), FadeOut(f_u1b), FadeOut(f_u2), FadeOut(f_u3),
            FadeOut(f_v0), FadeOut(f_v1), FadeOut(f_v1b), FadeOut(f_v2), FadeOut(f_v3),
            FadeIn(f_u1c), FadeIn(f_v1c))

        para = Polygon(PA, PB, PD, PC, fill_color=C_PARA, fill_opacity=0.3,
                       stroke_color=C_PARA, stroke_width=2)
        self.play(DrawBorderThenFill(para))
        self.bring_to_front(arr_u, arr_v, lbl_a_new)
        self.wait(3)

        # ── FRAME 8b: General 3D cross product formula ────────────────────────
        narr = narration(
            "The cross product of two 3D vectors gives a new perpendicular vector.", narr)

        v_bot = f_v1c.get_bottom()[1] - 0.48
        f_gen_hdr = T("General cross product formula for 3D vectors:", size=16, color=C_FORM)
        f_gen0    = T("u x v = (uy*vz - uz*vy,",                       size=17, color=C_FORM)
        f_gen1    = T("         uz*vx - ux*vz,",                        size=17, color=C_FORM)
        f_gen2    = T("         ux*vy - uy*vx)",                        size=17, color=C_FORM)
        place_col([f_gen_hdr, f_gen0, f_gen1, f_gen2], v_bot, gap=0.20)

        self.play(FadeIn(f_gen_hdr))
        self.play(FadeIn(f_gen0), FadeIn(f_gen1), FadeIn(f_gen2))
        self.wait(8)

        # ── FRAME 8c: Substitute z=0 ──────────────────────────────────────────
        narr = narration("Since z=0 for both vectors, most terms cancel.", narr)

        gen_bot = f_gen2.get_bottom()[1] - 0.60
        f_sub_hdr = T("Substituting 0 for all z components of u and v:", size=16, color=C_FORM)
        f_sub0    = T("u x v = (uy*0 - 0*vy,",                           size=17, color=C_FORM)
        f_sub1    = T("         0*vx - ux*0,",                            size=17, color=C_FORM)
        f_sub2    = T("         ux*vy - uy*vx)",                          size=17, color=C_FORM)
        place_col([f_sub_hdr, f_sub0, f_sub1, f_sub2], gen_bot, gap=0.20)

        self.play(FadeIn(f_sub_hdr))
        self.play(FadeIn(f_sub0), FadeIn(f_sub1), FadeIn(f_sub2))
        self.wait(7)

        # Collapse to clean 2D result
        narr = narration(
            "The result is a vector pointing purely along z — perpendicular to the plane.", narr)

        f_2d_hdr = T("General cross product formula for 2D vectors:", size=16, color=C_FORM)
        f_2d_fml = T("u x v = (0, 0, u.x*v.y - u.y*v.x)",             size=17, color=C_PARA)
        place_col([f_2d_hdr, f_2d_fml], v_bot, gap=0.22)

        self.play(
            FadeOut(f_gen_hdr), FadeOut(f_gen0), FadeOut(f_gen1), FadeOut(f_gen2),
            FadeOut(f_sub_hdr), FadeOut(f_sub0), FadeOut(f_sub1), FadeOut(f_sub2),
            FadeIn(f_2d_hdr), FadeIn(f_2d_fml))
        self.wait(7)

        # ── FRAME 9: Substitute values ────────────────────────────────────────
        narr = narration("Substituting u=(3,0,0) and v=(1,3,0):", narr)

        sub_bot = f_2d_fml.get_bottom()[1] - 0.60
        f_sub_lbl = T("Substituting:",                   size=16, color=C_FORM)
        f_calc0   = T("u x v = (0, 0, 3*3 - 0*1)",      size=17, color=C_FORM)
        f_calc1   = T("u x v = (0, 0, 9)",                size=17, color=C_PARA)
        f_calc2   = T("z = 9: vector points away from screen", size=16, color=C_PARA)
        place_col([f_sub_lbl, f_calc0, f_calc1], sub_bot, gap=0.22)
        f_calc2.move_to([FORM_LEFT, f_calc1.get_bottom()[1] - 0.55, 0], aligned_edge=LEFT)

        upper_centroid = (PB + PD + PC) / 3
        para_lbl1 = T("parallelogram", size=15, color=C_PARA_LBL, bold=True)\
                        .move_to(upper_centroid + UP * 0.18)
        para_lbl2 = T("area = 9", size=15, color=C_PARA_LBL, bold=True)\
                        .move_to(upper_centroid - UP * 0.10)

        self.play(FadeIn(f_sub_lbl))
        self.play(FadeIn(f_calc0))
        self.play(FadeIn(f_calc1))
        self.play(FadeIn(f_calc2))
        self.play(FadeIn(para_lbl1), FadeIn(para_lbl2))
        self.bring_to_front(para_lbl1, para_lbl2)
        self.wait(2)

        # ── FRAME 9b: Simulate 3D — fade grid, show z-arrow ──────────────────
        # We're a 2D Scene. To show the perpendicular vector we fade the grid,
        # keep the parallelogram, and draw an Arrow pointing UP from the centroid.
        # UP in screen space represents the +z direction coming toward the viewer.
        # A label makes this explicit.
        narr = narration(
            "The cross product vector is perpendicular to the plane — pointing away from the screen.",
            narr)

        self.play(FadeOut(plane), FadeOut(dash_a), FadeOut(dash_b), FadeOut(dash_c))

        z_arrow_pos = Arrow(
            PARA_CENTROID,
            PARA_CENTROID + UP * 2.76,
            color=C_ZVEC_POS,
            stroke_width=5,
            max_tip_length_to_length_ratio=0.15,
        )
        z_lbl_pos = T("+z: away from screen (CCW)", size=15, color=C_ZVEC_POS)\
                        .next_to(z_arrow_pos.get_tip(), RIGHT, buff=0.15)

        self.play(GrowArrow(z_arrow_pos), FadeIn(z_lbl_pos))
        self.bring_to_front(z_arrow_pos, z_lbl_pos)
        self.bring_to_front(para_lbl1, para_lbl2)
        self.wait(4)

        # ── FRAME 10: Triangle is half ────────────────────────────────────────
        narr = narration(
            "The z vector has length 9 — the parallelogram area. Triangle area is half: 4.5.",
            narr)

        tri        = Polygon(PA, PB, PC, fill_color=C_TRI, fill_opacity=0.5,
                             stroke_color=C_TRI, stroke_width=2)
        other_half = Polygon(PB, PD, PC, fill_color=GREY_D, fill_opacity=0.55,
                             stroke_width=0)
        diagonal   = DashedLine(PB, PC, color=WHITE, stroke_width=1.5)

        tri_centroid = (PA + PB + PC) / 3 + UP * 0.1
        tri_lbl1 = T("triangle", size=14, color=C_TRI_LBL, bold=True)\
                       .move_to(tri_centroid + UP * 0.18)
        tri_lbl2 = T("area = 4.5", size=14, color=C_TRI_LBL, bold=True)\
                       .move_to(tri_centroid - UP * 0.08)

        calc_bot = f_calc2.get_bottom()[1] - 0.26
        f_half = T("tri area = (1/2) * 9 = 4.5", size=17, color=C_TRI)
        place_col([f_half], calc_bot)

        self.play(Create(diagonal))
        self.play(DrawBorderThenFill(tri), FadeIn(other_half))
        self.bring_to_front(arr_u, arr_v, lbl_a_new)
        self.play(FadeIn(tri_lbl1), FadeIn(tri_lbl2))
        self.bring_to_front(tri_lbl1, tri_lbl2)
        self.play(FadeIn(f_half))
        self.wait(2.5)

        # ── FRAME 11a: CCW winding arrow ──────────────────────────────────────
        narr = narration(
            "a to b to c is counter-clockwise — cross product points away from the screen.", narr)

        u_side = PA + (PB - PA) * 0.36 + UP    * 0.15
        v_side = PA + (PC - PA) * 0.36 + RIGHT * 0.11

        # CCW: same arc bow as CW but reversed — tip on v-edge
        ccw_arrow = CurvedArrow(u_side, v_side, angle=PI / 2.5,
                                color=C_WIND, stroke_width=3, tip_length=0.16)
        ccw_lbl = T("ccw", size=16, color=C_WIND)\
                      .move_to(PA + RIGHT * 0.55 + DOWN * 0.50)

        # Final draw-order guarantee: labels and z-arrow above all fills
        self.bring_to_front(z_arrow_pos, z_lbl_pos, para_lbl1, para_lbl2, tri_lbl1, tri_lbl2, lbl_a_new)
        self.play(Create(ccw_arrow), FadeIn(ccw_lbl))
        self.wait(7)

        # ── FRAME 11b: CW — flip everything ──────────────────────────────────
        narr = narration(
            "Swap to v x u — z flips to -9, vector points into the screen.", narr)

        para_neg = Polygon(PA, PB, PD, PC, fill_color=C_PARA_NEG, fill_opacity=0.3,
                           stroke_color=C_PARA_NEG, stroke_width=2)
        tri_neg  = Polygon(PA, PB, PC, fill_color=C_PARA_NEG, fill_opacity=0.5,
                           stroke_color=C_PARA_NEG, stroke_width=2)

        # -z arrow points DOWN — into the screen
        z_arrow_neg = Arrow(
            PARA_CENTROID,
            PARA_CENTROID + DOWN * 3.31,
            color=C_ZVEC_NEG,
            stroke_width=5,
            max_tip_length_to_length_ratio=0.15,
        )
        z_lbl_neg = T("-z: into screen (CW)", size=15, color=C_ZVEC_NEG)\
                        .next_to(z_arrow_neg.get_tip(), RIGHT, buff=0.15)

        cw_arrow = CurvedArrow(v_side, u_side, angle=-PI / 2.5,
                               color=C_WIND, stroke_width=3, tip_length=0.16)
        cw_lbl = T("cw", size=16, color=C_WIND)\
                     .move_to(PA + RIGHT * 0.55 + DOWN * 0.50)

        para_neg_l1 = T("parallelogram", size=15, color=C_PARA_LBL, bold=True)\
                          .move_to(para_lbl1.get_center())
        para_neg_l2 = T("area = -9", size=15, color=C_PARA_LBL, bold=True)\
                          .move_to(para_lbl2.get_center())
        tri_neg_l1  = T("triangle", size=14, color=C_TRI_LBL, bold=True)\
                          .move_to(tri_lbl1.get_center())
        tri_neg_l2  = T("area = -4.5", size=14, color=C_TRI_LBL, bold=True)\
                          .move_to(tri_lbl2.get_center())

        half_bot = f_half.get_bottom()[1] - 0.55
        f_sw0 = T("Reversing order negates z:",  size=16, color=C_FORM)
        f_sw1 = T("v x u = (0, 0, 1*0 - 3*3)",  size=17, color=C_PARA_NEG)
        f_sw2 = T("v x u = (0, 0, -9)",           size=17, color=C_PARA_NEG)
        f_sw3 = T("z = -9: vector points into screen", size=16, color=C_PARA_NEG)
        place_col([f_sw0, f_sw1, f_sw2, f_sw3], half_bot, gap=0.22)

        self.play(
            ReplacementTransform(para,       para_neg),
            ReplacementTransform(tri,        tri_neg),
            ReplacementTransform(z_arrow_pos, z_arrow_neg),
            FadeOut(z_lbl_pos),
            ReplacementTransform(ccw_arrow,  cw_arrow),
            FadeOut(ccw_lbl),
            ReplacementTransform(para_lbl1,  para_neg_l1),
            ReplacementTransform(para_lbl2,  para_neg_l2),
            ReplacementTransform(tri_lbl1,   tri_neg_l1),
            ReplacementTransform(tri_lbl2,   tri_neg_l2),
        )
        self.play(FadeIn(z_lbl_neg), FadeIn(cw_lbl))
        self.bring_to_front(z_arrow_neg, z_lbl_neg, para_neg_l1, para_neg_l2, tri_neg_l1, tri_neg_l2, lbl_a_new)
        self.play(FadeIn(f_sw0))
        self.play(FadeIn(f_sw1), FadeIn(f_sw2))
        self.play(FadeIn(f_sw3))
        self.wait(7)

        # ── Final note ────────────────────────────────────────────────────────
        narr = narration(
            "Sign only has meaning if winding order and coordinate handedness are consistent.",
            narr)
        self.wait(3)
        self.play(FadeOut(narr))
        self.wait(0.5)
