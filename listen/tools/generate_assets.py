#!/usr/bin/env python3
"""Generate the quiz WAV assets with macOS TTS and ffmpeg.

The firmware reads unsigned 8-bit PCM, 8 kHz, mono WAV files. Run this script
on macOS where both `/usr/bin/say` and ffmpeg are available. ffmpeg can come
from Homebrew; `/opt/homebrew/bin/ffmpeg` is used when present.
"""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "assets"
ENGLISH_VOICE = "Samantha"
CHINESE_VOICE = "Tingting"
SAY = "/usr/bin/say"
FFMPEG = shutil.which("ffmpeg") or "/opt/homebrew/bin/ffmpeg"

QUESTIONS = [
    {
        "stem": "You'll hear: Where is the library? Question: What does the woman ask about?",
        "options": ["A place to read.", "A place to eat.", "A place to sleep.", "A place to swim."],
        "cn_stem": "你将听到：图书馆在哪里？问题：女士在询问什么？",
        "cn_options": ["一个读书的地方。", "一个吃饭的地方。", "一个睡觉的地方。", "一个游泳的地方。"],
    },
    {
        "stem": "You'll hear: I usually get up at six thirty. Question: What time does he get up?",
        "options": ["Six o'clock.", "Six fifteen.", "Six thirty.", "Seven thirty."],
        "cn_stem": "你将听到：我通常六点半起床。问题：他几点起床？",
        "cn_options": ["六点。", "六点十五。", "六点半。", "七点半。"],
    },
    {
        "stem": "You'll hear: My favorite subject is history because the stories are interesting. Question: Why does she like history?",
        "options": ["The teacher is kind.", "The stories are interesting.", "The class is easy.", "The homework is short."],
        "cn_stem": "你将听到：我最喜欢的科目是历史，因为故事很有趣。问题：她为什么喜欢历史？",
        "cn_options": ["老师很和善。", "故事很有趣。", "课程很简单。", "作业很少。"],
    },
    {
        "stem": "You'll hear: Let's take the bus. Parking near the museum is too difficult. Question: How will they travel?",
        "options": ["By bus.", "By bike.", "By car.", "On foot."],
        "cn_stem": "你将听到：我们坐公交车吧。博物馆附近停车太难了。问题：他们将怎样出行？",
        "cn_options": ["坐公交车。", "骑自行车。", "开汽车。", "步行。"],
    },
    {
        "stem": "You'll hear: It's rainy now, but the weather report says it will be sunny tomorrow. Question: What will tomorrow's weather be like?",
        "options": ["Rainy.", "Windy.", "Cloudy.", "Sunny."],
        "cn_stem": "你将听到：现在在下雨，但天气预报说明天会晴天。问题：明天天气怎样？",
        "cn_options": ["下雨。", "刮风。", "多云。", "晴天。"],
    },
    {
        "stem": "You'll hear: Could I have a glass of orange juice, please? Question: What does the man want?",
        "options": ["Milk.", "Water.", "Orange juice.", "Coffee."],
        "cn_stem": "你将听到：请给我一杯橙汁好吗？问题：男士想要什么？",
        "cn_options": ["牛奶。", "水。", "橙汁。", "咖啡。"],
    },
    {
        "stem": "You'll hear: The science museum is closed on Monday and opens at nine on other days. Question: When is the museum closed?",
        "options": ["On Monday.", "On Tuesday.", "On Saturday.", "On Sunday."],
        "cn_stem": "你将听到：科学博物馆周一闭馆，其他日子九点开门。问题：博物馆什么时候闭馆？",
        "cn_options": ["周一。", "周二。", "周六。", "周日。"],
    },
    {
        "stem": "You'll hear: Tom can't play basketball now because his left hand hurts. Question: Why can't Tom play basketball?",
        "options": ["He is too tired.", "His left hand hurts.", "He lost the ball.", "He has homework."],
        "cn_stem": "你将听到：汤姆现在不能打篮球，因为他的左手受伤了。问题：汤姆为什么不能打篮球？",
        "cn_options": ["他太累了。", "他的左手受伤了。", "他丢了球。", "他有作业。"],
    },
    {
        "stem": "You'll hear: The train to Beijing leaves in fifteen minutes. Question: How soon does the train leave?",
        "options": ["In five minutes.", "In ten minutes.", "In fifteen minutes.", "In fifty minutes."],
        "cn_stem": "你将听到：去北京的火车十五分钟后出发。问题：火车多久后出发？",
        "cn_options": ["五分钟后。", "十分钟后。", "十五分钟后。", "五十分钟后。"],
    },
    {
        "stem": "You'll hear: My sister wants to be a doctor and work in a children's hospital. Question: What does his sister want to be?",
        "options": ["A teacher.", "A doctor.", "A driver.", "A cook."],
        "cn_stem": "你将听到：我姐姐想当医生，在儿童医院工作。问题：他的姐姐想做什么工作？",
        "cn_options": ["教师。", "医生。", "司机。", "厨师。"],
    },
    {
        "stem": "You'll hear: This blue jacket is ninety yuan, and the black one is only seventy yuan. Question: How much is the black jacket?",
        "options": ["Sixty yuan.", "Seventy yuan.", "Eighty yuan.", "Ninety yuan."],
        "cn_stem": "你将听到：这件蓝色夹克九十元，黑色的只要七十元。问题：黑色夹克多少钱？",
        "cn_options": ["六十元。", "七十元。", "八十元。", "九十元。"],
    },
    {
        "stem": "You'll hear: Remember to take your umbrella. It may rain after lunch. Question: What should the man take?",
        "options": ["A hat.", "A coat.", "An umbrella.", "A camera."],
        "cn_stem": "你将听到：记得带伞。午饭后可能下雨。问题：男士应该带什么？",
        "cn_options": ["帽子。", "外套。", "雨伞。", "相机。"],
    },
    {
        "stem": "You'll hear: Lily is taller than Lucy, but Lucy runs faster. Question: Who runs faster?",
        "options": ["Lily.", "Lucy.", "Both run the same.", "Neither girl runs."],
        "cn_stem": "你将听到：莉莉比露西高，但露西跑得更快。问题：谁跑得更快？",
        "cn_options": ["莉莉。", "露西。", "两人一样快。", "两人都不跑。"],
    },
    {
        "stem": "You'll hear: I'd like noodles with beef, not rice with chicken. Question: What food does the woman order?",
        "options": ["Rice with chicken.", "Noodles with beef.", "Bread and eggs.", "Fish soup."],
        "cn_stem": "你将听到：我想要牛肉面，不是鸡肉饭。问题：女士点了什么食物？",
        "cn_options": ["鸡肉饭。", "牛肉面。", "面包和鸡蛋。", "鱼汤。"],
    },
    {
        "stem": "You'll hear: The school sports meeting will be on the playground this Friday afternoon. Question: Where will the sports meeting be?",
        "options": ["In the gym.", "In the park.", "On the playground.", "In the classroom."],
        "cn_stem": "你将听到：学校运动会本周五下午在操场举行。问题：运动会在哪里举行？",
        "cn_options": ["在体育馆。", "在公园。", "在操场。", "在教室。"],
    },
    {
        "stem": "You'll hear: David missed the early bus, so he was late for the meeting. Question: Why was David late?",
        "options": ["He overslept.", "He missed the bus.", "He lost his key.", "He met a friend."],
        "cn_stem": "你将听到：大卫错过了早班车，所以开会迟到了。问题：大卫为什么迟到？",
        "cn_options": ["他睡过头了。", "他错过了公交车。", "他丢了钥匙。", "他遇到了朋友。"],
    },
    {
        "stem": "You'll hear: Please turn off your phone before the film starts. Question: What should people do?",
        "options": ["Turn off the phone.", "Open the window.", "Leave the room.", "Buy a ticket."],
        "cn_stem": "你将听到：电影开始前请关掉手机。问题：人们应该做什么？",
        "cn_options": ["关掉手机。", "打开窗户。", "离开房间。", "买票。"],
    },
    {
        "stem": "You'll hear: Anna has lived in Shanghai for eight years, but she was born in Chengdu. Question: Where was Anna born?",
        "options": ["Shanghai.", "Chengdu.", "Beijing.", "Guangzhou."],
        "cn_stem": "你将听到：安娜在上海住了八年，但她出生在成都。问题：安娜出生在哪里？",
        "cn_options": ["上海。", "成都。", "北京。", "广州。"],
    },
    {
        "stem": "You'll hear: The bookstore is between the bank and the post office. Question: Where is the bookstore?",
        "options": ["Next to the school.", "Between the bank and the post office.", "Behind the hospital.", "Across from the hotel."],
        "cn_stem": "你将听到：书店在银行和邮局之间。问题：书店在哪里？",
        "cn_options": ["学校旁边。", "银行和邮局之间。", "医院后面。", "酒店对面。"],
    },
    {
        "stem": "You'll hear: We need two eggs, some milk, and a little sugar for the cake. Question: How many eggs do they need?",
        "options": ["One.", "Two.", "Three.", "Four."],
        "cn_stem": "你将听到：做蛋糕需要两个鸡蛋、一些牛奶和一点糖。问题：他们需要几个鸡蛋？",
        "cn_options": ["一个。", "两个。", "三个。", "四个。"],
    },
]

CN_DIGITS = ["零", "一", "二", "三", "四", "五", "六", "七", "八", "九"]


def chinese_number(value: int) -> str:
    if value == 100:
        return "一百"
    if value < 10:
        return CN_DIGITS[value]
    tens, ones = divmod(value, 10)
    if tens == 1:
        prefix = "十"
    else:
        prefix = CN_DIGITS[tens] + "十"
    return prefix + (CN_DIGITS[ones] if ones else "")


def required_files() -> dict[str, tuple[str, str]]:
    jobs: dict[str, tuple[str, str]] = {}

    for number in range(1, 21):
        jobs[f"ui_number_{number:02d}.wav"] = (CHINESE_VOICE, f"第{chinese_number(number)}题。")
    for option in range(1, 5):
        jobs[f"ui_option_{option}.wav"] = (CHINESE_VOICE, f"选项{chinese_number(option)}。")

    jobs["ui_listen.wav"] = (CHINESE_VOICE, "请说一、二、三、或者四。")
    jobs["ui_not_recognized.wav"] = (CHINESE_VOICE, "没有听清，请再试一次。")
    jobs["ui_correct.wav"] = (CHINESE_VOICE, "回答正确。")
    jobs["ui_wrong.wav"] = (CHINESE_VOICE, "回答错误。")
    for score in range(6):
        jobs[f"ui_score_{score}.wav"] = (CHINESE_VOICE, f"本题得{chinese_number(score)}分。")
    for total in range(101):
        jobs[f"ui_total_{total}.wav"] = (CHINESE_VOICE, f"总分数{chinese_number(total)}分。")

    for index, question in enumerate(QUESTIONS, 1):
        jobs[f"q{index:02d}_en_stem.wav"] = (ENGLISH_VOICE, question["stem"])
        jobs[f"q{index:02d}_cn_stem.wav"] = (CHINESE_VOICE, question["cn_stem"])
        for option, text in enumerate(question["options"], 1):
            jobs[f"q{index:02d}_en_opt{option}.wav"] = (ENGLISH_VOICE, text)
        for option, text in enumerate(question["cn_options"], 1):
            jobs[f"q{index:02d}_cn_opt{option}.wav"] = (CHINESE_VOICE, text)
    return jobs


def make_wav(name: str, voice: str, text: str, force: bool) -> bool:
    target = ASSETS / name
    if target.exists() and not force:
        return False
    target.parent.mkdir(parents=True, exist_ok=True)
    source = ASSETS / (name + ".source.wav")
    subprocess.run([SAY, "-v", voice, "--data-format=LEI16@22050", "-o", str(source), text], check=True)
    subprocess.run([
        FFMPEG, "-y", "-hide_banner", "-loglevel", "error", "-i", str(source),
        "-ar", "8000", "-ac", "1", "-c:a", "pcm_u8", str(target),
    ], check=True)
    source.unlink(missing_ok=True)
    return True


def verify_file(path: Path) -> tuple[int, int, int, int] | None:
    try:
        with wave.open(str(path), "rb") as wav:
            if wav.getcomptype() != "NONE" or wav.getsampwidth() != 1 or wav.getnchannels() != 1 or wav.getframerate() != 8000:
                return None
            return wav.getnframes(), 1, 1, 8000
    except (wave.Error, OSError):
        return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--force", action="store_true", help="regenerate existing WAV files")
    parser.add_argument("--check-only", action="store_true", help="only validate existing WAV files")
    args = parser.parse_args()

    if not Path(SAY).exists():
        parser.error(f"macOS say is required: {SAY}")
    if not Path(FFMPEG).exists():
        parser.error(f"ffmpeg is required: {FFMPEG}")

    jobs = required_files()
    ASSETS.mkdir(parents=True, exist_ok=True)
    if args.check_only:
        failures = [name for name in jobs if verify_file(ASSETS / name) is None]
        print(f"Checked {len(jobs)} expected assets; {len(failures)} invalid/missing.")
        if failures:
            for name in failures[:20]:
                print(f"  invalid: {name}")
            return 1
        print("All expected assets are valid 8 kHz mono u8 WAV files.")
        return 0

    generated = 0
    skipped = 0
    for number, (name, (voice, text)) in enumerate(jobs.items(), 1):
        if make_wav(name, voice, text, args.force):
            generated += 1
        else:
            skipped += 1
        if number % 20 == 0 or number == len(jobs):
            print(f"[{number}/{len(jobs)}] generated={generated} skipped={skipped}", flush=True)

    failures = [name for name in jobs if verify_file(ASSETS / name) is None]
    if failures:
        print(f"ERROR: {len(failures)} generated assets failed validation", file=sys.stderr)
        return 1
    print(f"Done: {generated} generated, {skipped} skipped, {len(jobs)} valid.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
