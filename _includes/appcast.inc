<?xml version="1.0" encoding="utf-8"?>
<rss version="2.0" xmlns:sparkle="http://www.andymatuschak.org/xml-namespaces/sparkle" xmlns:dc="http://purl.org/dc/elements/1.1/">
    <channel>
        <title>{{ site.github.project_title }}</title>
        <description>Most recent releases with links to updates.</description>
        <language>en</language>
        {% for release in site.github.releases %}
            {% unless release.draft %}
                {% unless release.prerelease and page.release_only %}
                    {% assign version = release.tag_name | remove_first:'v' %}
                    {% for asset in release.assets %}

                        {% if asset.name contains 'dmg' %}
                        {% assign os = 'osx' %}
                        {% elsif asset.name == 'win32' %}
                        {% assign os = 'windows-x86' %}
                        {% elsif asset.name == 'win64' %}
                        {% assign os = 'windows-x64' %}
                        {% endif %}

                        {% if os == 'osx' %}
                            <item>
                                <title>{{ release.name }}</title>
                                <description><![CDATA[{{ release.body | markdownify }}]]></description>
                                <pubDate>{{ release.published_at | date_to_rfc822 }}</pubDate>
                                <enclosure
                                    url="{{ asset.browser_download_url }}"
                                    sparkle:version="{{ version }}"
                                    length="{{ asset.size }}"
                                    type="application/octet-stream" />
                            </item>
                        {% endif %}
                    {%endfor %}
                {% endunless %}
            {% endunless %}
        {% endfor %}
    </channel>
</rss>
