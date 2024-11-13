// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.


const config = require('./config')
const fs = require('fs/promises');
const path = require('path')

// This function is used to modify build/config/unsafe_buffers_paths.txt, to
// address the issue that the Brave build uses the `-I../../..` flag, which
// allows `src/` files to be included. This alters how `clang` calculates the
// root of the source tree. With this function, all entries in
// unsafe_buffers_paths.txt are duplicated at the end with `src/` prepended to
// them.
async function updateUnsafeBuffersPaths() {
  const relativeBuffersPath = ['build', 'config', 'unsafe_buffers_paths.txt'];
  const chromiumRepoConfigPath =
      path.join(config.srcDir, ...relativeBuffersPath);
  const braveGenConfigPath =
      path.join(config.srcDir, ...['mises', ...relativeBuffersPath]);

  const repoContent =
      await fs.readFile(chromiumRepoConfigPath, {encoding: 'utf8'})

  // Adding exlusion paths for brave as well, which are also to be corrected
  // for src prefix
  const bufferPathLines = [
    '# File generated by updateUnsafeBuffersPaths.js. DO NOT edit manually.',
    '#',
    ...repoContent.split('\n'),
    '# Appended path correction for redirect_cc by update_patches.',
    '# File patched by brave/build/commands/lib/updateUnsafeBuffersPaths.js.',
    '-mises/'
  ];

  updatedPathLines = bufferPathLines.filter(
      (line) => line.startsWith('-') || line.startsWith('+'));
  updatedPathLines = updatedPathLines.map(
      (line) => {return line.slice(0, 1) + 'src/' + line.slice(1)});

  const updatedContents =
      [...bufferPathLines, ...updatedPathLines, ''].join('\n');

  // To avoid invalidating a lot of objects that might already have been built,
  // the file is only updated if there are any changes between the proposed and
  // current contents.
  try {
    currentFileContents =
        await fs.readFile(braveGenConfigPath, {encoding: 'utf8'});
  } catch (err) {
    // File doesn't exist
    currentFileContents = ''
  }
  if (currentFileContents !== updatedContents) {
    await fs.writeFile(braveGenConfigPath, updatedContents)
  }
}

module.exports = updateUnsafeBuffersPaths
