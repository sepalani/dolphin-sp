package org.dolphinemu.dolphinemu.adapters;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.drawable.Drawable;
import android.support.v17.leanback.widget.ImageCardView;
import android.support.v17.leanback.widget.Presenter;
import android.support.v4.app.FragmentActivity;
import android.support.v4.content.ContextCompat;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.Toast;

import org.dolphinemu.dolphinemu.R;
import org.dolphinemu.dolphinemu.model.GameFile;
import org.dolphinemu.dolphinemu.services.DirectoryInitializationService;
import org.dolphinemu.dolphinemu.ui.platform.Platform;
import org.dolphinemu.dolphinemu.ui.settings.SettingsActivity;
import org.dolphinemu.dolphinemu.utils.PicassoUtils;
import org.dolphinemu.dolphinemu.utils.SettingsFile;
import org.dolphinemu.dolphinemu.viewholders.TvGameViewHolder;

import java.io.File;

/**
 * The Leanback library / docs call this a Presenter, but it works very
 * similarly to a RecyclerView.Adapter.
 */
public final class GameRowPresenter extends Presenter
{
	@Override
	public ViewHolder onCreateViewHolder(ViewGroup parent)
	{
		// Create a new view.
		ImageCardView gameCard = new ImageCardView(parent.getContext());

		gameCard.setMainImageAdjustViewBounds(true);
		gameCard.setMainImageDimensions(480, 320);
		gameCard.setMainImageScaleType(ImageView.ScaleType.CENTER_CROP);

		gameCard.setFocusable(true);
		gameCard.setFocusableInTouchMode(true);

		// Use that view to create a ViewHolder.
		return new TvGameViewHolder(gameCard);
	}

	@Override
	public void onBindViewHolder(ViewHolder viewHolder, Object item)
	{
		TvGameViewHolder holder = (TvGameViewHolder) viewHolder;
		GameFile gameFile = (GameFile) item;

		holder.imageScreenshot.setImageDrawable(null);
		PicassoUtils.loadGameBanner(holder.imageScreenshot, gameFile);

		holder.cardParent.setTitleText(gameFile.getTitle());
		holder.cardParent.setContentText(gameFile.getCompany());

		holder.gameFile = gameFile;

		// Set the platform-dependent background color of the card
		int backgroundId;
		switch (Platform.fromNativeInt(gameFile.getPlatform()))
		{
			case GAMECUBE:
				backgroundId = R.drawable.tv_card_background_gamecube;
				break;
			case WII:
				backgroundId = R.drawable.tv_card_background_wii;
				break;
			case WIIWARE:
				backgroundId = R.drawable.tv_card_background_wiiware;
				break;
			default:
				throw new AssertionError("Not reachable.");
		}
		Context context = holder.cardParent.getContext();
		Drawable background = ContextCompat.getDrawable(context, backgroundId);
		holder.cardParent.setInfoAreaBackground(background);
		holder.cardParent.setOnLongClickListener(new View.OnLongClickListener() {
			@Override
			public boolean onLongClick(View view)
			{
				FragmentActivity activity = (FragmentActivity) view.getContext();
				String gameId = gameFile.getGameId();

				if (gameId.isEmpty())
				{
					AlertDialog.Builder builder = new AlertDialog.Builder(activity);
					builder.setTitle("Game Settings");
					builder.setMessage("Files without game IDs don't support game-specific settings.");

					builder.show();
					return true;
				}

				AlertDialog.Builder builder = new AlertDialog.Builder(activity);
				builder.setTitle("Game Settings")
					.setItems(R.array.gameSettingsMenus, new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int which) {
							switch (which) {
								case 0:
									SettingsActivity.launch(activity, SettingsFile.FILE_NAME_DOLPHIN, gameId);
									break;
								case 1:
									SettingsActivity.launch(activity, SettingsFile.FILE_NAME_GFX, gameId);
									break;
								case 2:
									String path = DirectoryInitializationService.getUserDirectory() + "/GameSettings/" + gameId + ".ini";
									File gameSettingsFile = new File(path);
									if (gameSettingsFile.exists())
									{
										if (gameSettingsFile.delete())
										{
											Toast.makeText(view.getContext(), "Cleared settings for " + gameId, Toast.LENGTH_SHORT).show();
										}
										else
										{
											Toast.makeText(view.getContext(), "Unable to clear settings for " + gameId, Toast.LENGTH_SHORT).show();
										}
									}
									else
									{
										Toast.makeText(view.getContext(), "No game settings to delete", Toast.LENGTH_SHORT).show();
									}
									break;
							}
						}
					});

				builder.show();
				return true;
			}
		});
	}


	@Override
	public void onUnbindViewHolder(ViewHolder viewHolder)
	{
		// no op
	}
}
