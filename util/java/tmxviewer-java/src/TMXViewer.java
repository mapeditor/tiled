import tiled.core.Map;
import tiled.io.TMXMapReader;

import javax.swing.*;

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

        JFrame appFrame = new JFrame("TMX Viewer");
        appFrame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        appFrame.setSize(800, 600);
        appFrame.setVisible(true);

        TMXMapReader mapReader = new TMXMapReader();
        try {
            Map map = mapReader.readMap(fileToOpen);
            System.out.println(map.toString() + " loaded");
        } catch (Exception e) {
            System.out.println("Error while reading the map:\n" + e.getMessage());
        }
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
