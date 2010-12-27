import tiled.core.Map;
import tiled.core.MapLayer;
import tiled.core.TileLayer;
import tiled.io.TMXMapReader;
import tiled.view.MapRenderer;
import tiled.view.OrthogonalRenderer;

import javax.swing.*;
import java.awt.*;

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
    private Map map;
    private MapRenderer renderer;

    public MapView(Map map) {
        this.map = map;
        renderer = createRenderer(map);

        setPreferredSize(renderer.getMapSize());
        setOpaque(true);
    }

    public void paintComponent(Graphics g) {
        final Graphics2D g2d = (Graphics2D) g.create();
        final Rectangle clip = g2d.getClipBounds();

        // Draw a gray background
        g2d.setPaint(new Color(100, 100, 100));
        g2d.fill(clip);

        // Draw each tile map layer
        for (MapLayer layer : map) {
            if (layer instanceof TileLayer) {
                renderer.paintTileLayer(g2d, (TileLayer) layer);
            }
        }
    }

    private static MapRenderer createRenderer(Map map) {
        switch (map.getOrientation()) {
            case Map.ORIENTATION_ORTHOGONAL:
                return new OrthogonalRenderer(map);
            default:
                return null;
        }
    }

    public Dimension getPreferredScrollableViewportSize() {
        return getPreferredSize();
    }

    public int getScrollableUnitIncrement(Rectangle visibleRect,
                                          int orientation, int direction) {
        if (orientation == SwingConstants.HORIZONTAL)
            return map.getTileWidth();
        else
            return map.getTileHeight();
    }

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

    public boolean getScrollableTracksViewportWidth() {
        return false;
    }

    public boolean getScrollableTracksViewportHeight() {
        return false;
    }
}