package com.rogach;

import java.awt.BorderLayout;
import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.util.List;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JSlider;

/**
 * The only Swing code: a frame, a panel drawing the rendered image,
 * a heading slider at the bottom (0-360, initial 180) and a pitch
 * slider at the side (-90-90, initial 0). Slider changes repaint.
 * The window is the primary output; it is NOT covered by automated
 * tests (a window cannot be checked headlessly) — see TEST_PLAN.
 */
public final class DemoViewer {
    private final JFrame frame;
    private final JSlider headingSlider;
    private final JSlider pitchSlider;
    private final List<Triangle> triangles;
    private BufferedImage image;

    public DemoViewer(List<Triangle> triangles) {
        this.triangles = triangles;
        frame = new JFrame("3D Render Engine");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(400, 400);
        JPanel panel = new JPanel() {
            @Override
            protected void paintComponent(Graphics g) {
                super.paintComponent(g);
                if (image != null) {
                    g.drawImage(image, 0, 0, null);
                }
            }
        };
        headingSlider = new JSlider(0, 360, 180);
        pitchSlider = new JSlider(JSlider.VERTICAL, -90, 90, 0);
        headingSlider.addChangeListener(e -> updateImage(panel));
        pitchSlider.addChangeListener(e -> updateImage(panel));
        frame.add(panel, BorderLayout.CENTER);
        frame.add(headingSlider, BorderLayout.SOUTH);
        frame.add(pitchSlider, BorderLayout.EAST);
        updateImage(panel);
    }

    private void updateImage(JPanel panel) {
        image = Renderer.render(triangles, headingSlider.getValue(),
                pitchSlider.getValue(), panel.getWidth() > 0 ? panel.getWidth() : 400,
                panel.getHeight() > 0 ? panel.getHeight() : 400);
        panel.repaint();
    }

    public void show() {
        frame.setVisible(true);
    }
}
