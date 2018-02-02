package com.righere.convexdplayer;

import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

import com.righere.convexdplayer.sdl.SDLActivity;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private Button btnPlay;
    private Button btnPause;
    private Button btnStart;
    private Button btnNext;
    private Button btnStop;
    private Button btnSeek;
    private TextView tvInfo;
    private TextView tvTime;
    private TextView tvLog;
    private String log = "";
    private String urlTitle = "英雄联盟";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        btnPlay = (Button) findViewById(R.id.btn_play);
        btnPause = (Button) findViewById(R.id.btn_pause);
        tvInfo = (TextView) findViewById(R.id.tv_info);
        btnStart = (Button) findViewById(R.id.btn_start);
        btnNext = (Button) findViewById(R.id.btn_next);
        btnStop = (Button) findViewById(R.id.btn_stop);
        btnSeek = (Button) findViewById(R.id.btn_seek);
        tvTime = (TextView) findViewById(R.id.tv_time);
        tvLog = (TextView) findViewById(R.id.tv_log);
        String url = "zui.mp3";
//		url = "jxtg3.mkv";
        final String input = new File(Environment.getExternalStorageDirectory(),url).getAbsolutePath();


        SDLActivity.initPlayer();
        SDLActivity.setDataSource(input);
        //WlPlayer.initSurface(surface);
        SDLActivity.prePard();

        SDLActivity.setPrepardListener(new SDLActivity.OnPlayerPrepard() {

            @Override
            public void onPrepard() {
                // TODO Auto-generated method stub
                //WlPlayer.wlSeekTo(2650);
                SDLActivity.wlStart();
                handler.sendEmptyMessage(1);

            }
        });

        SDLActivity.setOnCompleteListener(new SDLActivity.OnCompleteListener() {

            @Override
            public void onConplete() {
                // TODO Auto-generated method stub
                System.out.println("...........complete..............");
                log = "log:--->\n complete";
                handler.sendEmptyMessage(1);
            }
        });

        SDLActivity.setOnErrorListener(new SDLActivity.OnErrorListener() {

            @Override
            public void onError(int code, String msg) {
                // TODO Auto-generated method stub
                System.out.println("code:" + code + ",msg:" + msg);
                log = "log:--->\n" + msg;
                handler.sendEmptyMessage(1);
            }
        });


        handler.postDelayed(runnable, 1000);

        SDLActivity.setOnPlayerInfoListener(new SDLActivity.OnPlayerInfoListener() {

            @Override
            public void onPlay() {
                // TODO Auto-generated method stub
                System.out.println("...........play..............");
                log = "playing";
                handler.sendEmptyMessage(1);
            }

            @Override
            public void onLoad() {
                // TODO Auto-generated method stub
                System.out.println("...........load..............");
                log = "loading";
                handler.sendEmptyMessage(1);
            }
        });

        SDLActivity.wlStart();
        handler.sendEmptyMessage(1);
        btnStart.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                log = "start";
                handler.sendEmptyMessage(1);
                SDLActivity.wlStart();
            }
        });

        btnPlay.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                log = "start";
                handler.sendEmptyMessage(1);
                SDLActivity.wlPlay();
            }
        });

        btnPause.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                // TODO Auto-generated method stub
                log = "pause";
                handler.sendEmptyMessage(1);
                SDLActivity.wlPause();
            }
        });

        btnStop.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                log = "stop";
                handler.sendEmptyMessage(1);
                SDLActivity.release();

            }
        });

        btnNext.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                urlTitle = "中国之声";
                //WlPlayer.next("http://cnvod.cnr.cn/audio2017/live/zgzs/201707/qlglx_20170711021143zgzs_h.m4a");
                SDLActivity.next(input);
            }
        });

        btnSeek.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                // TODO Auto-generated method stub
                int ret = SDLActivity.wlSeekTo(3600);
                System.out.println("ret:" + ret);
            }
        });

    }


    Handler handler = new Handler()
    {
        public void handleMessage(android.os.Message msg) {
            tvInfo.setText("time:" + millisToDateFormat(SDLActivity.wlDuration()));
            tvLog.setText("播放：" + urlTitle + "\n状态：" + log);
        };
    };



    Runnable runnable = new Runnable() {

        @Override
        public void run() {
            // TODO Auto-generated method stub
            tvTime.setText(millisToDateFormat(SDLActivity.wlNowTime()));
            handler.postDelayed(runnable, 1000);
        }
    };


    public String millisToDateFormat(int sends) {
        long hours = sends / (60 * 60);
        long minutes = (sends % (60 * 60)) / (60);
        long seconds = sends % (60);

        String sh = "00";
        if (hours > 0) {
            if (hours < 10) {
                sh = "0" + hours;
            } else {
                sh = hours + "";
            }
        }
        String sm = "00";
        if (minutes > 0) {
            if (minutes < 10) {
                sm = "0" + minutes;
            } else {
                sm = minutes + "";
            }
        }

        String ss = "00";
        if (seconds > 0) {
            if (seconds < 10) {
                ss = "0" + seconds;
            } else {
                ss = seconds + "";
            }
        }
        return sh + ":" + sm + ":" + ss;
    }

    @Override
    public void onBackPressed() {
        // TODO Auto-generated method stub
        SDLActivity.release();
        super.onBackPressed();
    }
}
