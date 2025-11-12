with (import <nixpkgs> {});
mkShell {
  buildInputs = [ ruby bundler ];
  shellHook = ''
    bundle config set path 'vendor/bundle'
    bundle install
    bundle exec jekyll serve --drafts --livereload
  '';
}
