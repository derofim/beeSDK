package com.sohu.tv.edu.main.activity;


import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.Editable;
import android.text.InputFilter;
import android.text.TextWatcher;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.sohu.tv.edu.R;
import com.sohu.tv.edu.base.ui.ClearableEditTextWithIcon;

/**
 * A login screen that offers login via email/password.
 */
public class LoginActivity extends AppCompatActivity {
    private Button loginBtn;
    private Button registerBtn;
    private TextView switchModeBtn;  // 注册/登录切换按钮

    private ClearableEditTextWithIcon loginAccountEdit;
    private ClearableEditTextWithIcon loginPasswordEdit;

    private ClearableEditTextWithIcon registerAccountEdit;
    private ClearableEditTextWithIcon registerNickNameEdit;
    private ClearableEditTextWithIcon registerPasswordEdit;

    private View loginLayout;
    private View registerLayout;

    private boolean registerMode = false; // 注册模式



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        toolbar.setLogo(R.drawable.actionbar_logo_white);
        toolbar.setTitle(R.string.app_name);
        setSupportActionBar(toolbar);

        setupLoginPanel();
    }


    /**
     * 登录面板
     */
    private void setupLoginPanel() {
        loginAccountEdit = (ClearableEditTextWithIcon) findViewById(R.id.edit_login_account);
        loginPasswordEdit = (ClearableEditTextWithIcon) findViewById(R.id.edit_login_password);
        loginBtn = (Button) findViewById(R.id.done);

        loginAccountEdit.setIconResource(R.drawable.user_account_icon);
        loginPasswordEdit.setIconResource(R.drawable.user_pwd_lock_icon);

        loginAccountEdit.setFilters(new InputFilter[]{new InputFilter.LengthFilter(32)});
        loginPasswordEdit.setFilters(new InputFilter[]{new InputFilter.LengthFilter(32)});
        loginAccountEdit.addTextChangedListener(textWatcher);
        loginPasswordEdit.addTextChangedListener(textWatcher);

    //    String account = AuthPreferences.getUserAccount();
    //    loginAccountEdit.setText(account);

        loginBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                login();
            }
        });
    }

    private TextWatcher textWatcher = new TextWatcher() {

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {

        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {

        }

        @Override
        public void afterTextChanged(Editable s) {
            // 更新右上角按钮状态
            if (!registerMode) {
                // 登录模式
                boolean isEnable = loginAccountEdit.getText().length() > 0
                        && loginPasswordEdit.getText().length() > 0;
        //        updateBtn(loginBtn, isEnable);
            }
        }
    };

    private void login() {
        startActivity(new Intent(LoginActivity.this, CreateRoomActivity.class));
        finish();
    }

}

