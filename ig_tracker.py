import instaloader
import json
import os
from datetime import datetime

def load_previous_data(username):
    filename = f"{username}_history.json"
    if os.path.exists(filename):
        with open(filename, 'r') as f:
            return json.load(f)
    return None

def save_current_data(username, data):
    filename = f"{username}_history.json"
    with open(filename, 'w') as f:
        json.dump(data, f, indent=4)

def fetch_and_compare(username):
    L = instaloader.Instaloader()
    
    # --- AUTOMATIC LOGIN BY COOKIES ---
    try:
        print("[+] Loading browser cookies to bypass 403 blocks...")
        # This looks for an active Instagram login session in Chrome, Firefox, or Edge
        L.load_session_from_file(username) # Checks if a local session is already saved
    except FileNotFoundError:
        try:
            # If no session file exists, grab cookies from your default browser
            # You can change 'chrome' to 'firefox' or 'edge' if you use a different browser!
            L.load_session_from_browser(username, browser="chrome") 
            print("[+] Successfully authenticated using browser cookies!")
        except Exception as e:
            print(f"[!] Warning: Could not import browser session ({e}). Trying anonymously...")

    print(f"[+] Fetching live data for: {username}...")
    
    try:
        profile = instaloader.Profile.from_username(L.context, username)
        
        current_data = {
            "username": profile.username,
            "followers": profile.followers,
            "following": profile.followees,
            "posts": profile.mediacount,
            "bio": profile.biography,
            "profile_pic": profile.profile_pic_url,
            "last_checked": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        }
        
        old_data = load_previous_data(username)
        
        if old_data is None:
            print(f"\n[!] First time tracking {username}. Saving initial snapshot!")
            print(f"Followers: {current_data['followers']} | Posts: {current_data['posts']}")
        else:
            print("\n================= CHANGE LOG =================")
            print(f"Checking updates since: {old_data['last_checked']}")
            
            # 1. Track Followers Change
            follower_diff = current_data['followers'] - old_data['followers']
            if follower_diff > 0:
                print(f"[📈] Followers: Increased by +{follower_diff} (Total: {current_data['followers']})")
            elif follower_diff < 0:
                print(f"[📉] Followers: Decreased by {follower_diff} (Total: {current_data['followers']})")
            else:
                print("[•] Followers: No change")
                
            # 2. Track Posts Change
            post_diff = current_data['posts'] - old_data['posts']
            if post_diff > 0:
                print(f"[📸] Posts: {post_diff} new post(s) added!")
            elif post_diff < 0:
                print(f"[🗑️] Posts: {abs(post_diff)} post(s) deleted.")
                
            # 3. Track Bio Change
            if current_data['bio'] != old_data['bio']:
                print("[📝] Bio Changed!")
                print(f"    Old: {old_data['bio']}")
                print(f"    New: {current_data['bio']}")
                
            # 4. Track Profile Picture Change
            if current_data['profile_pic'] != old_data['profile_pic']:
                print("[🖼️] Profile Picture was updated!")
                
            print("==============================================")
            
        save_current_data(username, current_data)
        
    except Exception as e:
        print(f"[-] Error: {e}")

if __name__ == "__main__":
    # Wrap username in quotes to avoid syntax errors!
    target_account = "1nsane_bitchhh" 
    fetch_and_compare(target_account)