package com.Madgine;

import android.view.*; 
import android.os.*;
import android.app.*;

public class MadgineActivity extends NativeActivity {
  View getNativeActivityView() {
    // This is hacky as hell, but NativeActivity does not give any proper way of
    // accessing it.
    ViewGroup parent = (ViewGroup) (getWindow().getDecorView());
    parent = (ViewGroup) parent.getChildAt(0);
    parent = (ViewGroup) parent.getChildAt(1);
    return parent.getChildAt(0);
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    View nativeActivityView = getNativeActivityView();
    nativeActivityView.setFocusable(true);
    nativeActivityView.setFocusableInTouchMode(true);
    nativeActivityView.requestFocus();
  }
}