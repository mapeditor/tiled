with (import <nixpkgs> {});
let
  gh-pages = ruby.withPackages (ps: with ps; [
    github-pages
    jekyll-redirect-from
    webrick
  ]);
in
  mkShell {
    buildInputs = [ gh-pages ];
    shellHook = ''
      jekyll serve --drafts --livereload
    '';
  }
