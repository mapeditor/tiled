# tiled-api

This package includes TypeScript definitions for the Tiled scripting API which
(depending on your editor support) also allow you to write simple JavaScript
scripts with automatic code completion.

## Online documentation

See: https://www.mapeditor.org/docs/scripting

## Writing a Tiled plugin with code completion in VS Code

Create a new project:

```bash
mkdir example-tiled-ts-plugin
cd example-tiled-ts-plugin
npm init
```

Install the Tiled definitions:
```bash
npm install @mapeditor/tiled-api --save-dev
```

Write your plugin and enjoy code-completion:
```js
/// <reference types="@mapeditor/tiled-api" />

const action = tiled.registerAction("CustomAction", function(action) {
    tiled.log(action.text + " was " + (action.checked ? "checked" : "unchecked"))
})

action.text = "My Custom Action"
action.checkable = true
action.shortcut = "Ctrl+K"

tiled.extendMenu("Edit", [
    { action: "CustomAction", before: "SelectAll" },
    { separator: true }
]);
```

Of course, when writing TypeScript code instead of JavaScript you also get
compilation errors instead of only syntax highlighting.
