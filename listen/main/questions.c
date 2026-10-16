#include "questions.h"

static const question_t k_questions[QUESTION_COUNT] = {
    {
        "You'll hear: Where is the library? Question: What does the woman ask about?",
        {"A place to read.", "A place to eat.", "A place to sleep.", "A place to swim."},
        "你将听到：图书馆在哪里？问题：女士在询问什么？",
        {"一个读书的地方。", "一个吃饭的地方。", "一个睡觉的地方。", "一个游泳的地方。"},
        0,
    },
    {
        "You'll hear: I usually get up at six thirty. Question: What time does he get up?",
        {"Six o'clock.", "Six fifteen.", "Six thirty.", "Seven thirty."},
        "你将听到：我通常六点半起床。问题：他几点起床？",
        {"六点。", "六点十五。", "六点半。", "七点半。"},
        2,
    },
    {
        "You'll hear: My favorite subject is history because the stories are interesting. Question: Why does she like history?",
        {"The teacher is kind.", "The stories are interesting.", "The class is easy.", "The homework is short."},
        "你将听到：我最喜欢的科目是历史，因为故事很有趣。问题：她为什么喜欢历史？",
        {"老师很和善。", "故事很有趣。", "课程很简单。", "作业很少。"},
        1,
    },
    {
        "You'll hear: Let's take the bus. Parking near the museum is too difficult. Question: How will they travel?",
        {"By bus.", "By bike.", "By car.", "On foot."},
        "你将听到：我们坐公交车吧。博物馆附近停车太难了。问题：他们将怎样出行？",
        {"坐公交车。", "骑自行车。", "开汽车。", "步行。"},
        0,
    },
    {
        "You'll hear: It's rainy now, but the weather report says it will be sunny tomorrow. Question: What will tomorrow's weather be like?",
        {"Rainy.", "Windy.", "Cloudy.", "Sunny."},
        "你将听到：现在在下雨，但天气预报说明天会晴天。问题：明天天气怎样？",
        {"下雨。", "刮风。", "多云。", "晴天。"},
        3,
    },
    {
        "You'll hear: Could I have a glass of orange juice, please? Question: What does the man want?",
        {"Milk.", "Water.", "Orange juice.", "Coffee."},
        "你将听到：请给我一杯橙汁好吗？问题：男士想要什么？",
        {"牛奶。", "水。", "橙汁。", "咖啡。"},
        2,
    },
    {
        "You'll hear: The science museum is closed on Monday and opens at nine on other days. Question: When is the museum closed?",
        {"On Monday.", "On Tuesday.", "On Saturday.", "On Sunday."},
        "你将听到：科学博物馆周一闭馆，其他日子九点开门。问题：博物馆什么时候闭馆？",
        {"周一。", "周二。", "周六。", "周日。"},
        0,
    },
    {
        "You'll hear: Tom can't play basketball now because his left hand hurts. Question: Why can't Tom play basketball?",
        {"He is too tired.", "His left hand hurts.", "He lost the ball.", "He has homework."},
        "你将听到：汤姆现在不能打篮球，因为他的左手受伤了。问题：汤姆为什么不能打篮球？",
        {"他太累了。", "他的左手受伤了。", "他丢了球。", "他有作业。"},
        1,
    },
    {
        "You'll hear: The train to Beijing leaves in fifteen minutes. Question: How soon does the train leave?",
        {"In five minutes.", "In ten minutes.", "In fifteen minutes.", "In fifty minutes."},
        "你将听到：去北京的火车十五分钟后出发。问题：火车多久后出发？",
        {"五分钟后。", "十分钟后。", "十五分钟后。", "五十分钟后。"},
        2,
    },
    {
        "You'll hear: My sister wants to be a doctor and work in a children's hospital. Question: What does his sister want to be?",
        {"A teacher.", "A doctor.", "A driver.", "A cook."},
        "你将听到：我姐姐想当医生，在儿童医院工作。问题：他的姐姐想做什么工作？",
        {"教师。", "医生。", "司机。", "厨师。"},
        1,
    },
    {
        "You'll hear: This blue jacket is ninety yuan, and the black one is only seventy yuan. Question: How much is the black jacket?",
        {"Sixty yuan.", "Seventy yuan.", "Eighty yuan.", "Ninety yuan."},
        "你将听到：这件蓝色夹克九十元，黑色的只要七十元。问题：黑色夹克多少钱？",
        {"六十元。", "七十元。", "八十元。", "九十元。"},
        1,
    },
    {
        "You'll hear: Remember to take your umbrella. It may rain after lunch. Question: What should the man take?",
        {"A hat.", "A coat.", "An umbrella.", "A camera."},
        "你将听到：记得带伞。午饭后可能下雨。问题：男士应该带什么？",
        {"帽子。", "外套。", "雨伞。", "相机。"},
        2,
    },
    {
        "You'll hear: Lily is taller than Lucy, but Lucy runs faster. Question: Who runs faster?",
        {"Lily.", "Lucy.", "Both run the same.", "Neither girl runs."},
        "你将听到：莉莉比露西高，但露西跑得更快。问题：谁跑得更快？",
        {"莉莉。", "露西。", "两人一样快。", "两人都不跑。"},
        1,
    },
    {
        "You'll hear: I'd like noodles with beef, not rice with chicken. Question: What food does the woman order?",
        {"Rice with chicken.", "Noodles with beef.", "Bread and eggs.", "Fish soup."},
        "你将听到：我想要牛肉面，不是鸡肉饭。问题：女士点了什么食物？",
        {"鸡肉饭。", "牛肉面。", "面包和鸡蛋。", "鱼汤。"},
        1,
    },
    {
        "You'll hear: The school sports meeting will be on the playground this Friday afternoon. Question: Where will the sports meeting be?",
        {"In the gym.", "In the park.", "On the playground.", "In the classroom."},
        "你将听到：学校运动会本周五下午在操场举行。问题：运动会在哪里举行？",
        {"在体育馆。", "在公园。", "在操场。", "在教室。"},
        2,
    },
    {
        "You'll hear: David missed the early bus, so he was late for the meeting. Question: Why was David late?",
        {"He overslept.", "He missed the bus.", "He lost his key.", "He met a friend."},
        "你将听到：大卫错过了早班车，所以开会迟到了。问题：大卫为什么迟到？",
        {"他睡过头了。", "他错过了公交车。", "他丢了钥匙。", "他遇到了朋友。"},
        1,
    },
    {
        "You'll hear: Please turn off your phone before the film starts. Question: What should people do?",
        {"Turn off the phone.", "Open the window.", "Leave the room.", "Buy a ticket."},
        "你将听到：电影开始前请关掉手机。问题：人们应该做什么？",
        {"关掉手机。", "打开窗户。", "离开房间。", "买票。"},
        0,
    },
    {
        "You'll hear: Anna has lived in Shanghai for eight years, but she was born in Chengdu. Question: Where was Anna born?",
        {"Shanghai.", "Chengdu.", "Beijing.", "Guangzhou."},
        "你将听到：安娜在上海住了八年，但她出生在成都。问题：安娜出生在哪里？",
        {"上海。", "成都。", "北京。", "广州。"},
        1,
    },
    {
        "You'll hear: The bookstore is between the bank and the post office. Question: Where is the bookstore?",
        {"Next to the school.", "Between the bank and the post office.", "Behind the hospital.", "Across from the hotel."},
        "你将听到：书店在银行和邮局之间。问题：书店在哪里？",
        {"学校旁边。", "银行和邮局之间。", "医院后面。", "酒店对面。"},
        1,
    },
    {
        "You'll hear: We need two eggs, some milk, and a little sugar for the cake. Question: How many eggs do they need?",
        {"One.", "Two.", "Three.", "Four."},
        "你将听到：做蛋糕需要两个鸡蛋、一些牛奶和一点糖。问题：他们需要几个鸡蛋？",
        {"一个。", "两个。", "三个。", "四个。"},
        1,
    },
};

const question_t *questions_get(size_t index)
{
    if (index >= QUESTION_COUNT) {
        return NULL;
    }
    return &k_questions[index];
}

const char *questions_get_language(void)
{
    return "English listening quiz";
}

size_t questions_text_count(void)
{
    return sizeof(k_questions) / sizeof(k_questions[0]);
}
