package com.Madgine;

import android.app.Activity;

import com.google.android.gms.games.*; 
import com.google.android.gms.games.leaderboard.*; 
import com.google.android.gms.tasks.*; 
import java.util.*;


class VoidReceiver implements OnCompleteListener{
	long ptr;

	private native static void setValue(long ptr);
	private native static void setError(long ptr, Exception ex);

	public VoidReceiver(long ptr){
		this.ptr = ptr;
	}

	public void onComplete(Task task){
		if (task.isSuccessful()){
			setValue(ptr);
		}else{
			setError(ptr, task.getException());
		}
	}
}

class ScoresReceiver implements OnCompleteListener{
	long ptr;

	private native static void addScore(long ptr, long id, String name, long rank, int score);
	private native static void setValue(long ptr);
	private native static void setError(long ptr, Exception ex);

	public ScoresReceiver(long ptr){
		this.ptr = ptr;
	}

	public void onComplete(Task task){
		if (task.isSuccessful()){
			LeaderboardsClient.LeaderboardScores scores = ((AnnotatedData<LeaderboardsClient.LeaderboardScores>)task.getResult()).get();
			for(LeaderboardScore score : scores.getScores()){
				addScore(ptr, 1, score.getScoreHolderDisplayName(), score.getRank(), (int)score.getRawScore());
			}
			setValue(ptr);
			scores.release();
		}else{
			setError(ptr, task.getException());
		}
	}
}

public class PlayServices {

	private static Activity activity;
	private static AchievementsClient achievements;
	private static LeaderboardsClient leaderboards;

	public static void Setup(Activity _activity)
	{
		activity = _activity;
		achievements = PlayGames.getAchievementsClient(activity);
		leaderboards = PlayGames.getLeaderboardsClient(activity);
	}

	public static String toId(String name){
		return name.replaceAll("\\(","").replaceAll("\\)","").replaceAll(" ", "_").toLowerCase();
	}

	public static void Achievements_Unlock(String name, long receiver) 
	{
		String fullName = "achievement_" + toId(name);
		String id = getStringResourceByName(fullName);
		Task<Void> task = achievements.unlockImmediate(id);
		task.addOnCompleteListener(new VoidReceiver(receiver));
	}

	public static void Leaderboards_SubmitScore(String leaderboardName, int score, String name, long receiver) 
	{
		String fullName = "leaderboard_" + toId(leaderboardName);
		String id = getStringResourceByName(fullName);
		Task<ScoreSubmissionData> task = leaderboards.submitScoreImmediate(id, score, name);
		task.addOnCompleteListener(new VoidReceiver(receiver));
	}

	public static void Leaderboards_GetLeaderboard(String leaderboardName, int accessmode, int referenceRank, int rangeBegin, int rangeEnd, long receiver)
	{
		String fullName = "leaderboard_" + toId(leaderboardName);
		String id = getStringResourceByName(fullName);
		Task<AnnotatedData<LeaderboardsClient.LeaderboardScores>> task;
		int collection;
		if (accessmode == 0){ //FRIENDS
			collection = LeaderboardVariant.COLLECTION_FRIENDS;
		}else{ //GLOBAL
			collection = LeaderboardVariant.COLLECTION_PUBLIC;
		}
		if (referenceRank == 0){ //USER
			task = leaderboards.loadPlayerCenteredScores(id,  LeaderboardVariant.TIME_SPAN_ALL_TIME, collection, rangeEnd - rangeBegin + 1, true);
		}else{ //FIRST
			task = leaderboards.loadTopScores(id,  LeaderboardVariant.TIME_SPAN_ALL_TIME, collection, rangeEnd - rangeBegin + 1, true);
		}
		task.addOnCompleteListener(new ScoresReceiver(receiver));
	}
	
	private static String getStringResourceByName(String aString) {
      String packageName = activity.getPackageName();
      int resId = activity.getResources().getIdentifier(aString, "string", packageName);
      return activity.getString(resId);
    }
}