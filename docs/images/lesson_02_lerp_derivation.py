from manim import *

FONT          = "Courier New"
COLOR_BG      = "#1e1e2e"
COLOR_POINT   = "#79c0ff"
COLOR_EDGE    = "#e2e8f0"
COLOR_DX      = "#4f8ef7"
COLOR_SMALL   = "#f47f7f"
COLOR_ITER    = "#3fb950"
COLOR_T       = "#c4b5fd"
COLOR_FORMULA = "#c4b5fd"


def T(s, size=20, color=COLOR_EDGE):
    return Text(s, font=FONT, font_size=size, color=color)


class LerpDerivation(Scene):
    def construct(self):
        self.camera.background_color = COLOR_BG

        A = np.array([-4.0, -1.8, 0])
        B = np.array([ 3.5,  1.8, 0])
        x_steps  = 14
        step_vec = (B - A) / x_steps
        corner   = np.array([B[0], A[1], 0])

        ix0, iy0 = 2,  10
        ix1, iy1 = 16, 17
        idx = ix1 - ix0
        idy = iy1 - iy0
        stop_at_ix = 13

        # ── Points present at start ───────────────────────────────────────
        dot_A     = Dot(A, radius=0.1, color=COLOR_POINT)
        dot_B     = Dot(B, radius=0.1, color=COLOR_POINT)
        lbl_A_sym = T("(x0, y0)", size=20, color=COLOR_POINT)\
                       .next_to(dot_A, LEFT, buff=0.15)
        lbl_B_sym = T("(x1, y1)", size=20, color=COLOR_POINT)\
                       .next_to(dot_B, UP, buff=0.35)
        lbl_A_num = T(f"({ix0}, {iy0})", size=18, color=COLOR_POINT)\
                       .next_to(lbl_A_sym, DOWN, buff=0.08)
        lbl_B_num = T(f"({ix1}, {iy1})", size=18, color=COLOR_POINT)\
                       .next_to(lbl_B_sym, DOWN, buff=0.08)
        lbl_A_num.set_opacity(0)
        lbl_B_num.set_opacity(0)

        self.add(dot_A, dot_B, lbl_A_sym, lbl_B_sym, lbl_A_num, lbl_B_num)
        self.wait(1.2)

        # ── Line morphs in ────────────────────────────────────────────────
        edge = Line(A, B, color=COLOR_EDGE, stroke_width=2.5)
        self.play(Create(edge))
        self.wait(0.8)

        # ── dx and dy — no numbers yet ────────────────────────────────────
        dx_line = Line(A, corner, color=COLOR_DX, stroke_width=2.0).shift(DOWN * 0.55)
        lbl_dx  = T("dx = x1 - x0", size=20, color=COLOR_DX).next_to(dx_line, DOWN, buff=0.14)
        dy_line = Line(corner, B, color=COLOR_DX, stroke_width=2.0).shift(RIGHT * 0.45)
        lbl_dy  = T("dy = y1 - y0", size=20, color=COLOR_DX).next_to(dy_line, RIGHT, buff=0.14)

        self.play(Create(dx_line), FadeIn(lbl_dx))
        self.play(Create(dy_line), FadeIn(lbl_dy))
        self.wait(0.8)

        large_tri = Polygon(A, corner, B,
                            fill_color=COLOR_DX, fill_opacity=0.15,
                            stroke_color=COLOR_DX, stroke_width=1.2)
        self.play(FadeIn(large_tri))
        self.wait(0.6)

        # ── Small triangle ────────────────────────────────────────────────
        i        = 5
        cur      = A + i * step_vec
        cur_base = np.array([cur[0], A[1], 0])

        seg_x = Line(A, cur_base, color=COLOR_SMALL, stroke_width=2.5)
        seg_y = Line(cur_base, cur, color=COLOR_SMALL, stroke_width=2.5)

        # x - x0 label: below the horizontal segment, centered on it
        lbl_xx = T("x - x0", size=15, color=COLOR_SMALL)\
                     .next_to(seg_x, DOWN, buff=0.12)
        lbl_yy = T("y - y0", size=15, color=COLOR_SMALL)\
                     .next_to(seg_y, RIGHT, buff=0.12)
        dot_cur    = Dot(cur, radius=0.09, color=COLOR_ITER)
        lbl_xy_dot = T("(x, y)", size=20, color=COLOR_ITER).next_to(dot_cur, UP, buff=0.5)

        self.play(Create(seg_x), FadeIn(lbl_xx))
        self.play(Create(seg_y), FadeIn(lbl_yy))
        self.play(FadeIn(dot_cur), FadeIn(lbl_xy_dot))
        self.wait(1.0)

        # ── Top-left derivation ───────────────────────────────────────────
        header = T("y = y0 + t * dy  derivation", size=24, color=COLOR_FORMULA)\
                     .to_edge(UP, buff=0.35)
        header_center_x = header.get_center()[0]

        # anchor for left-aligned derivation rows — true top-left corner
        corner_ref = T("X", size=19).to_corner(UL, buff=0.4)
        ul_x = corner_ref.get_left()[0]
        ul_y = header.get_bottom()[1] - 0.55

        def row(text, size=19, color=COLOR_EDGE, offset=0):
            lbl = T(text, size=size, color=color)
            lbl.move_to([ul_x + lbl.width / 2, ul_y - offset, 0])
            return lbl

        def hint(text, ref_lbl, offset=0.42):
            lbl = T(text, size=15, color=COLOR_EDGE)
            lbl.next_to(ref_lbl, DOWN, buff=offset)
            lbl.align_to(ref_lbl, LEFT)
            return lbl

        self.play(FadeIn(header))
        self.wait(0.8)

        eq1 = row("dy / dx = (y - y0) / (x - x0)")
        self.play(FadeIn(eq1))
        self.wait(2.5)

        h1 = hint("cross multiply:", eq1)
        self.play(FadeIn(h1))
        self.wait(1.5)
        eq2 = row("(x - x0) / dx = (y - y0) / dy")
        self.play(FadeOut(eq1), FadeOut(h1))
        self.play(FadeIn(eq2))
        self.wait(2.5)

        h2 = hint("let  t = (x - x0) / dx", eq2)
        self.play(FadeIn(h2))
        self.wait(2.0)
        eq3 = row("t = (y - y0) / dy")
        self.play(FadeOut(eq2), FadeOut(h2))
        self.play(FadeIn(eq3))
        self.wait(2.5)

        h3 = hint("multiply both sides by dy:", eq3)
        self.play(FadeIn(h3))
        self.wait(1.5)
        eq4 = row("t * dy = y - y0")
        self.play(FadeOut(eq3), FadeOut(h3))
        self.play(FadeIn(eq4))
        self.wait(2.5)

        h4 = hint("add y0 to both sides:", eq4)
        self.play(FadeIn(h4))
        self.wait(1.5)
        eq5 = row("y0 + t * dy = y")
        self.play(FadeOut(eq4), FadeOut(h4))
        self.play(FadeIn(eq5))
        self.wait(2.0)

        eq6 = row("y = y0 + t * dy", color=COLOR_FORMULA)
        box = SurroundingRectangle(eq6, color=COLOR_FORMULA,
                                   buff=0.15, corner_radius=0.08, stroke_width=1.5)
        self.play(FadeOut(eq5))
        self.play(FadeIn(eq6), Create(box))
        self.wait(3.0)

        # ── Stepping — numeric substitution ───────────────────────────────
        box_bottom_y = eq6.get_bottom()[1]
        t_num_y = box_bottom_y - 0.65
        y_num_y = box_bottom_y - 1.15

        def make_num(text, y_pos, color):
            lbl = T(text, size=18, color=color)
            lbl.move_to([ul_x + lbl.width / 2, y_pos, 0])
            return lbl

        lbl_t_num = make_num(" ", t_num_y, COLOR_T)
        lbl_y_num = make_num(" ", y_num_y, COLOR_FORMULA)
        self.add(lbl_t_num, lbl_y_num)

        pixels     = VGroup()
        first_step = True
        self.add(pixels)

        for step in range(i, stop_at_ix - ix0 + 1):
            nix = ix0 + step
            nt  = (nix - ix0) / idx
            niy = iy0 + nt * idy

            next_pt   = A + step * step_vec
            next_base = np.array([next_pt[0], A[1], 0])

            new_seg_x = Line(A, next_base, color=COLOR_SMALL, stroke_width=2.5)
            new_seg_y = Line(next_base, next_pt, color=COLOR_SMALL, stroke_width=2.5)
            new_dot   = Dot(next_pt, radius=0.09, color=COLOR_ITER)

            seg_mid_x = (A[0] + next_base[0]) / 2
            new_lbl_xx = T(f"x-x0 = {nix}-{ix0} = {nix-ix0}",
                           size=15, color=COLOR_SMALL)\
                             .move_to([seg_mid_x, A[1] - 0.28, 0])
            new_lbl_yy = T(f"y-y0 = {niy:.1f}-{iy0} = {niy-iy0:.1f}",
                           size=15, color=COLOR_SMALL)\
                             .next_to(new_seg_y, RIGHT, buff=0.12)

            new_t_num = make_num(f"t = ({nix}-{ix0})/{idx} = {nt:.2f}",
                                 t_num_y, COLOR_T)
            new_y_num = make_num(f"y = {iy0}+{nt:.2f}*{idy} = {niy:.1f}",
                                 y_num_y, COLOR_FORMULA)

            new_xy = T(f"({nix}, {niy:.1f})", size=24, color=COLOR_ITER)\
                        .next_to(new_dot, UP, buff=0.5)

            if first_step:
                new_lbl_dx = T(f"dx = x1 - x0 = {idx}", size=20, color=COLOR_DX)\
                                 .next_to(dx_line, DOWN, buff=0.14)
                new_lbl_dy = T(f"dy = y1 - y0 = {idy}", size=20, color=COLOR_DX)\
                                 .next_to(dy_line, RIGHT, buff=0.14)
                self.play(
                    Transform(seg_x,   new_seg_x),
                    Transform(seg_y,   new_seg_y),
                    Transform(dot_cur, new_dot),
                    Transform(lbl_dx,  new_lbl_dx),
                    Transform(lbl_dy,  new_lbl_dy),
                    lbl_A_num.animate.set_opacity(1),
                    lbl_B_num.animate.set_opacity(1),
                    run_time=1.0
                )
                self.remove(lbl_t_num, lbl_y_num, lbl_xx, lbl_yy, lbl_xy_dot)
                lbl_t_num, lbl_y_num = new_t_num, new_y_num
                lbl_xx, lbl_yy, lbl_xy_dot = new_lbl_xx, new_lbl_yy, new_xy
                self.add(lbl_t_num, lbl_y_num, lbl_xx, lbl_yy, lbl_xy_dot)
                first_step = False
                self.wait(3.0)
            else:
                self.play(
                    Transform(seg_x,   new_seg_x),
                    Transform(seg_y,   new_seg_y),
                    Transform(dot_cur, new_dot),
                    run_time=1.5
                )
                self.remove(lbl_t_num, lbl_y_num, lbl_xx, lbl_yy, lbl_xy_dot)
                lbl_t_num, lbl_y_num = new_t_num, new_y_num
                lbl_xx, lbl_yy, lbl_xy_dot = new_lbl_xx, new_lbl_yy, new_xy
                self.add(lbl_t_num, lbl_y_num, lbl_xx, lbl_yy, lbl_xy_dot)
                self.wait(0.5)

            pixels.add(Dot(next_pt, radius=0.07, color=COLOR_ITER))
            self.add(pixels)

        self.wait(2.0)
