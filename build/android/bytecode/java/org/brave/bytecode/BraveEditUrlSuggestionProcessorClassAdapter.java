/* Copyright (c) 2020 The Mises Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mises.bytecode;

import org.objectweb.asm.ClassVisitor;

public class MisesEditUrlSuggestionProcessorClassAdapter extends MisesClassVisitor {
    static String sEditUrlSuggestionProcessor =
            "org/chromium/chrome/browser/omnibox/suggestions/editurl/EditUrlSuggestionProcessor";
    static String sMisesEditUrlSuggestionProcessor =
            "org/chromium/chrome/browser/omnibox/suggestions/editurl/MisesEditUrlSuggestionProcessor";

    public MisesEditUrlSuggestionProcessorClassAdapter(ClassVisitor visitor) {
        super(visitor);

        redirectConstructor(sEditUrlSuggestionProcessor, sMisesEditUrlSuggestionProcessor);

        deleteField(sMisesEditUrlSuggestionProcessor, "mHasClearedOmniboxForFocus");
        makeProtectedField(sEditUrlSuggestionProcessor, "mHasClearedOmniboxForFocus");
    }
}
