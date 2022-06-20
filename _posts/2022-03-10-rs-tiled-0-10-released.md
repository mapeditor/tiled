---
layout: post
title: rs-tiled, the official Rust loader for Tiled!
author:
  name: Alejandro Perea
  twitter: aleokstudios
tags: rs-tiled
---

Hello! I'm Alejandro Perea, core maintainer of the [`tiled` crate](https://crates.io/crates/tiled), a Rust library to load Tiled maps and tilesets.

Back in December 2021, [@mattyhall]'s `rs-tiled` was [moved into the @mapeditor organization][adoption-issue] on Github. The move was done with the intention of gathering several maintainers and making it officially supported. There are now three more maintainers: [@aleokdev] (me), [@bjorn] (creator of Tiled) and [@PieKing1215].

Since then, we've been figuring out ways to improve the interface of the library and provide a more modern, "rustier" approach to it, while also supporting more Tiled features. Today, we've released version [0.10.0][release-notes] of the crate, our first big milestone.

If you were a past user of the crate, we *highly recommend* that you check out the new version. You can use the new [examples] for porting old code over.

I'd like to thank [@mattyhall] for having maintained this crate for the last 7 years and for letting us continue the work of maintaining it, Thorbjørn for all the code reviews and feedback provided, and of course all the contributors that have joined us and users who have provided feedback and bug reports!

If you are interested in the full changelist since `rs-tiled` 0.9.5, you can go over the [release notes on GitHub][release-notes].

[@PieKing1215]: https://github.com/PieKing1215
[@bjorn]: https://github.com/bjorn
[@mattyhall]: https://github.com/mattyhall
[@aleokdev]: https://github.com/aleokdev
[adoption-issue]: https://github.com/mapeditor/rs-tiled/issues/105
[release-notes]: https://github.com/mapeditor/rs-tiled/releases/v0.10.0
[examples]: https://github.com/mapeditor/rs-tiled/tree/master/examples
[@mapeditor]: https://github.com/mapeditor
