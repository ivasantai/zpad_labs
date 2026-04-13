import streamlit as st
import pandas as pd
import plotly.express as px

st.set_page_config(page_title="Lab 5", layout="wide")

st.title("Лабораторна робота №5")
st.subheader("Наука про дані: обмін результатами та початковий аналіз")

@st.cache_data
def load_data():
    return pd.read_csv("vhi_cleaned_all.csv")

df = load_data()

metric_options = ["VCI", "TCI", "VHI"]
region_col = "region_id"
year_col = "year"
week_col = "week"

min_year, max_year = int(df[year_col].min()), int(df[year_col].max())
min_week, max_week = int(df[week_col].min()), int(df[week_col].max())

regions = sorted(df[region_col].dropna().unique().tolist())


col1, col2 = st.columns([1, 2])

with col1:
    st.header("Фільтри")

    metric = st.selectbox("Оберіть ряд", metric_options, index=2)
    region = st.selectbox("Оберіть область", regions)

    year_range = st.slider(
        "Інтервал років",
        min_year,
        max_year,
        (min_year, max_year)
    )

    week_range = st.slider(
        "Інтервал тижнів",
        min_week,
        max_week,
        (min_week, max_week)
    )

    sort_asc = st.checkbox("Сортувати за зростанням")
    sort_desc = st.checkbox("Сортувати за спаданням")

    reset = st.button("Reset")

if reset:
    st.rerun()

filtered_df = df[
    (df[year_col] >= year_range[0]) &
    (df[year_col] <= year_range[1]) &
    (df[week_col] >= week_range[0]) &
    (df[week_col] <= week_range[1])
].copy()

region_df = filtered_df[filtered_df[region_col] == region].copy()

if sort_asc and not sort_desc:
    region_df = region_df.sort_values(by=metric, ascending=True)
elif sort_desc and not sort_asc:
    region_df = region_df.sort_values(by=metric, ascending=False)

with col2:
    tab1, tab2, tab3 = st.tabs(["Таблиця", "Графік 1", "Графік 2"])

    with tab1:
        st.subheader("Дані")
        st.dataframe(region_df, use_container_width=True)

    with tab2:
        st.subheader(f"{metric} для області {region}")

        fig1 = px.line(
            region_df,
            x=week_col,
            y=metric,
            color=year_col,
            title=f"{metric} по тижнях"
        )

        st.plotly_chart(fig1, use_container_width=True)

    with tab3:
        st.subheader("Порівняння областей")

        compare_df = filtered_df.groupby(region_col, as_index=False)[metric].mean()

        fig2 = px.bar(
            compare_df,
            x=region_col,
            y=metric,
            title=f"Середнє значення {metric} по областях"
        )

        st.plotly_chart(fig2, use_container_width=True)

st.markdown("### Інструкція")
st.write("""
1. Оберіть показник (VCI, TCI або VHI)
2. Оберіть область
3. Встановіть інтервал років і тижнів
4. Використовуйте сортування при потребі
5. Перейдіть між вкладками для перегляду таблиці та графіків
6. Кнопка Reset скидає параметри
""")