/*-
 * #%L
 * This file is part of tmxviewer-java.
 * %%
 * Copyright (C) 2010 - 2020 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2016 - 2020 Mike Thomas <mikepthomas@outlook.com>
 * %%
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * #L%
 */
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.Rectangle;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.Scrollable;
import javax.swing.SwingConstants;
import javax.swing.Timer;
import javax.swing.WindowConstants;

import org.mapeditor.core.Group;
import org.mapeditor.core.Map;
import org.mapeditor.core.ObjectGroup;
import org.mapeditor.core.MapLayer;
import org.mapeditor.core.TileLayer;
import org.mapeditor.io.TMXMapReader;
import org.mapeditor.view.HexagonalRenderer;
import org.mapeditor.view.MapRenderer;
import org.mapeditor.view.ObliqueRenderer;
import org.mapeditor.view.OrthogonalRenderer;
import org.mapeditor.view.IsometricRenderer;


/**
 * An example showing how to use libtiled-java to do a simple TMX viewer.
 */
public class TMXViewer
{
    public static void main(String[] arguments) {
        String fileToOpen = null;

        for (String arg : arguments) {
            if ("-?".equals(arg) || "-help".equals(arg)) {
                printHelpMessage();
                return;
            } else if (arg.startsWith("-")) {
                System.out.println("Unknown option: " + arg);
                printHelpMessage();
                return;
            } else if (fileToOpen == null) {
                fileToOpen = arg;
            }
        }

        if (fileToOpen == null) {
            printHelpMessage();
            return;
        }

        Map map;
        try {
            TMXMapReader mapReader = new TMXMapReader();
            map = mapReader.readMap(fileToOpen);
        } catch (Exception e) {
            System.out.println("Error while reading the map:\n" + e.getMessage());
            return;
        }

        System.out.println(map.toString() + " loaded");

        JScrollPane scrollPane = new JScrollPane(new MapView(map));
        scrollPane.setBorder(null);
        scrollPane.setPreferredSize(new Dimension(800, 600));

        JFrame appFrame = new JFrame("TMX Viewer");
        appFrame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        appFrame.setContentPane(scrollPane);
        appFrame.pack();
        appFrame.setVisible(true);
    }

    private static void printHelpMessage() {
        System.out.println("Java TMX Viewer\n" +
                "\n" +
                "When a parameter is given, it can either be a file name or an \n" +
                "option starting with '-'. These options are available:\n" +
                "\n" +
                "-?\n" +
                "-help\n" +
                "\tDisplays this help message\n");
    }
}

class MapView extends JPanel implements Scrollable
{
    private final Map map;
    private final MapRenderer renderer;
    private final Timer animationTimer;
    private final int originOffsetX;
    private final int originOffsetY;

    public MapView(Map map) {
        this.map = map;
        renderer = createRenderer(map);

        // For infinite maps, layer bounds can be negative.
        // Compute an origin offset so everything shifts into positive pixel space.
        if (map.getInfinite() != null && map.getInfinite() == 1) {
            int minX = 0, minY = 0;
            for (int i = 0; i < map.getLayerCount(); i++) {
                Rectangle b = map.getLayer(i).getBounds();
                minX = Math.min(minX, b.x);
                minY = Math.min(minY, b.y);
            }
            originOffsetX = -minX * map.getTileWidth();
            originOffsetY = -minY * map.getTileHeight();
        } else {
            originOffsetX = 0;
            originOffsetY = 0;
        }

        setPreferredSize(renderer.getMapSize());
        setOpaque(true);

        animationTimer = new Timer(33, e -> repaint());
        animationTimer.start();
    }

    @Override
    public void paintComponent(Graphics g) {
        final Graphics2D g2d = (Graphics2D) g.create();
        final Rectangle clip = g2d.getClipBounds();
        g2d.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
        g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);

        // Draw a gray background
        g2d.setPaint(new Color(100, 100, 100));
        g2d.fill(clip);

        // Shift so negative tile coordinates appear in positive pixel space
        g2d.translate(originOffsetX, originOffsetY);

        // Draw each map layer
        paintLayers(g2d, map.getLayers());
        g2d.dispose();
    }

    private void paintLayers(Graphics2D g2d, java.util.List<MapLayer> layers) {
        for (MapLayer layer : layers) {
            final Graphics2D layerGraphics = (Graphics2D) g2d.create();
            try {
                applyParallaxTranslation(layerGraphics, layer);

                if (layer instanceof Group) {
                    paintLayers(layerGraphics, ((Group) layer).getLayers());
                } else if (layer instanceof TileLayer) {
                    renderer.paintTileLayer(layerGraphics, (TileLayer) layer);
                } else if (layer instanceof ObjectGroup) {
                    renderer.paintObjectGroup(layerGraphics, (ObjectGroup) layer);
                }
            } finally {
                layerGraphics.dispose();
            }
        }
    }

    private void applyParallaxTranslation(Graphics2D g2d, MapLayer layer) {
        final double parallaxOriginX = map.getParallaxoriginx() != null ? map.getParallaxoriginx() : 0.0;
        final double parallaxOriginY = map.getParallaxoriginy() != null ? map.getParallaxoriginy() : 0.0;
        final double parallaxX = layer.getParallaxx() != null ? layer.getParallaxx() : 1.0;
        final double parallaxY = layer.getParallaxy() != null ? layer.getParallaxy() : 1.0;

        final int translateX = (int) Math.round(parallaxOriginX * (1.0 - parallaxX));
        final int translateY = (int) Math.round(parallaxOriginY * (1.0 - parallaxY));
        g2d.translate(translateX, translateY);
    }

    private static MapRenderer createRenderer(Map map) {
        switch (map.getOrientation()) {
            case ORTHOGONAL:
                return new OrthogonalRenderer(map);

            case ISOMETRIC:
                return new IsometricRenderer(map);

            case STAGGERED:
            case HEXAGONAL:
                return new HexagonalRenderer(map);

            case OBLIQUE:
                return new ObliqueRenderer(map);

            default:
                return null;
        }
    }

    @Override
    public Dimension getPreferredScrollableViewportSize() {
        return getPreferredSize();
    }

    @Override
    public int getScrollableUnitIncrement(Rectangle visibleRect,
                                          int orientation, int direction) {
        if (orientation == SwingConstants.HORIZONTAL)
            return map.getTileWidth();
        else
            return map.getTileHeight();
    }

    @Override
    public int getScrollableBlockIncrement(Rectangle visibleRect,
                                           int orientation, int direction) {
        if (orientation == SwingConstants.HORIZONTAL) {
            final int tileWidth = map.getTileWidth();
            return (visibleRect.width / tileWidth - 1) * tileWidth;
        } else {
            final int tileHeight = map.getTileHeight();
            return (visibleRect.height / tileHeight - 1) * tileHeight;
        }
    }

    @Override
    public boolean getScrollableTracksViewportWidth() {
        return false;
    }

    @Override
    public boolean getScrollableTracksViewportHeight() {
        return false;
    }
}
