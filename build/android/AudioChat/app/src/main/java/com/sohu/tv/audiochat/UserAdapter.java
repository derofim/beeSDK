package com.sohu.tv.audiochat;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.List;

public class UserAdapter extends ArrayAdapter<UserModel> {
    private int resourceId;

    private static final int [] volumeResIds = {
            R.drawable.volume_ic_1,
            R.drawable.volume_ic_2,
            R.drawable.volume_ic_3,
            R.drawable.volume_ic_4,
            R.drawable.volume_ic_5,
            R.drawable.volume_ic_6,
            R.drawable.volume_ic_7,
            R.drawable.volume_ic_8
    };

    public UserAdapter(Context context, int resource, List<UserModel> objects) {
        super(context, resource, objects);
        resourceId = resource;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        UserModel userItem = getItem(position);

        View userItemView = LayoutInflater.from(getContext()).inflate(resourceId, null);

        String showName = userItem.nickName;
        ImageView identityImage = (ImageView)userItemView.findViewById(R.id.identity_image);
        if (userItem.creator) {
            identityImage.setImageResource(R.drawable.master_icon);
            showName += "(房主)";
        }

        if (userItem.local) {
            showName += "(本地)";
        }

        CircleImageView avatarImageView = (CircleImageView)userItemView.findViewById(R.id.user_head);
        avatarImageView.setImageResource(R.drawable.ic_default_avatar);

        TextView nameText = (TextView)userItemView.findViewById(R.id.user_name);
        nameText.setText(showName);

        ImageView volumeImageView = (ImageView)userItemView.findViewById(R.id.volume_image);
        int volume = userItem.volume < 0 ? 0 : userItem.volume;
        if (volume == 0) {
            volumeImageView.setImageDrawable(null);
        } else {
            volume = volume > 7 ? 7 : volume;
            int volumeResId = volumeResIds[volume];
            volumeImageView.setImageResource(volumeResId);
        }

        return userItemView;
    }
}
